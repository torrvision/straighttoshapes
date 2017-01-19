#include "detection_layer.h"
#include "activations.h"
#include "softmax_layer.h"
#include "blas.h"
#include "box.h"
#include "cuda.h"
#include "utils.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

detection_layer make_detection_layer(int batchSize, int inputs, int n, int side, int classes, int coords, int shapeparams, int rescore)
{
    detection_layer l = {0};
    l.type = DETECTION;

    l.n = n;
    l.batch = batchSize;
    l.inputs = inputs;
    l.classes = classes;
    l.coords = coords;
    l.shapeparams = shapeparams;
    l.rescore = rescore;
    l.side = side;
    assert(side*side*((1 + l.coords + l.shapeparams)*l.n + l.classes) == inputs);
    l.cost = calloc(1, sizeof(float));
    l.outputs = l.inputs;
    l.truths = l.side*l.side*(1+l.coords+l.shapeparams+l.classes);
    l.output = calloc(batchSize*l.outputs, sizeof(float));
    l.delta = calloc(batchSize*l.outputs, sizeof(float));
#ifdef WITH_CUDA
    l.output_gpu = cuda_make_array(l.output, batchSize*l.outputs);
    l.delta_gpu = cuda_make_array(l.delta, batchSize*l.outputs);
#endif

    fprintf(stderr, "Detection Layer\n");

    return l;
}

void forward_detection_layer(const detection_layer l, network_state state)
{
  // state.input -> pointer to input data.
  // state.truth -> pointer to target data.
  //
  int cellCount = l.side*l.side;
  int batchSize = l.batch;
  int boxesPerCell = l.n;
  const float noObjectTarget = 0.0f;
  const float trueObjectTarget = 1.0f;

  int i,j;
  // Copy the state input to the layer output.
  memcpy(l.output, state.input, l.outputs*batchSize*sizeof(float));

  int batch;
  if (l.softmax)
  {
    for(batch = 0; batch < batchSize; ++batch)
    {
      int index = batch*l.inputs;
      for (i = 0; i < cellCount; ++i)
      {
        int offset = i*l.classes;
        softmax_array(l.output + index + offset, l.classes, 1,
                      l.output + index + offset);
      }
    }
  }

  if(state.train)
  {
    // Initialise variables.
    float avg_iou = 0;
    float avg_shape_iou = 0;
    float avg_shape_sq_err = 0;
    float avgPrecitedProbTrueCategories = 0;
    float avgPredictedProbAllCategories = 0;
    float avgBoxConfidenceScore = 0;
    float avgAnyBoxConfidenceScore = 0;
    int count = 0;
    *(l.cost) = 0;
    int size = l.inputs * batchSize;
    memset(l.delta, 0, size * sizeof(float));

    // Do processing on each batch.
    for(batch = 0; batch < batchSize; ++batch)
    {
      int index = batch*l.inputs; // Index into array per batch

      // Prediction array:
      // [ (.. prob mass fns ..)(.. box confidences ..)(.. box-shapes ..) ]
      // [ ( pmf cell1<pmf1> )( conf cell1<conf1,conf2..> cell2<..> )( boxes cell1<boxshape1,boxshape2..> cell2<..>)
      // [ ( cellCount * categoryCount )( cellCount * boxesPerCell )( cellCount * (4coords + shapeparams) * boxesPerCell) ]

      for(i = 0; i < cellCount; ++i) 
      {
        // The first element is the box confidence score,
        // The ground truth is 1 if there is an object.
        int truth_index = (batch * cellCount + i) * ( 1 + l.coords + l.shapeparams + l.classes);
        int is_obj = state.truth[truth_index];

        // l.n -> The number of boxes each cell is responsible for.
        for(j = 0; j < boxesPerCell; ++j)
        {
          // Box confidence index in the prediction array.
          // The confidence prediction represents the IOU between the predicted box and any ground truth box.
          int confIndex = index + cellCount*l.classes + i*boxesPerCell + j;
          float boxConfidence = l.output[confIndex];

          // TODO: come back to this.
          float diff = noObjectTarget - boxConfidence;
          l.delta[confIndex] = l.noobject_scale * diff; // Box Confidence Term.
          //*(l.cost) += l.noobject_scale * (diff*diff);  // Box Confidence Term.
          avgAnyBoxConfidenceScore += boxConfidence;
        }

        if(!is_obj){
            continue;
        }

        // Pass over the conditional class probabilities.
        int class_index = index + i*l.classes;
        for(j = 0; j < l.classes; ++j)
        {
          float delta = (state.truth[truth_index + 1 + j] - l.output[class_index + j]);

          l.delta[class_index+j] = l.class_scale * delta; // Probability Term.

          //*(l.cost) += l.class_scale * pow(delta, 2);     // Probability Term.

          // FIXME: float comparison
          float classProbGivenObject = l.output[class_index + j];
          if(state.truth[truth_index + 1 + j]) avgPrecitedProbTrueCategories += classProbGivenObject;
          avgPredictedProbAllCategories += classProbGivenObject;
        }

        // Pass over the boxes.
        int tbox_index = truth_index + 1 + l.classes;
        box truth = float_to_box(state.truth + tbox_index);

        // Normalise the coordinate so that 0-1 range is all within one cell.
        truth.x /= l.side;
        truth.y /= l.side;

        int best_index = -1;
        float best_iou = 0;
        float best_rmse = 20;

        // Find the predicted box which best overlaps with the groud truth.
        for(j = 0; j < boxesPerCell; ++j)
        {
          int box_index = index + cellCount*(l.classes + boxesPerCell) + (i*boxesPerCell + j) * (l.coords + l.shapeparams);
          // Pointer to the first element of the box, it will only process 4 elements.
          box out = float_to_box(l.output + box_index);

          // Normalise the coordinate so that 0-1 range is all within one cell.
          out.x /= l.side;
          out.y /= l.side;

          // Square the sides since the network predicts the sqrt of the width and height.
          if(l.sqrt)
          {
            out.w = out.w*out.w;
            out.h = out.h*out.h;
          }

          float iou  = box_iou(out, truth);
          float rmse = box_rmse(out, truth);
          if(best_iou > 0.0f || iou > 0.0f)
          {
            if(iou > best_iou)
            {
              best_iou = iou;
              best_index = j;
            }
          }
          else
          {
            if(rmse < best_rmse)
            {
              best_rmse = rmse;
              best_index = j;
            }
          }
        }

#if 0
        if(l.random && *(state.net.seen) < 64000)
        {
          best_index = rand()%boxesPerCell;
        }
#endif
        //printf("%d,", best_index);

        int box_index = index + cellCount*(l.classes + boxesPerCell) + (i*boxesPerCell + best_index) * (l.coords + l.shapeparams);

        box out = float_to_box(l.output + box_index);
        out.x /= l.side;
        out.y /= l.side;
        if(l.sqrt)
        {
          out.w = out.w*out.w;
          out.h = out.h*out.h;
        }
        float iou = box_iou(out, truth);

        int confIndex = index + cellCount*l.classes + i*boxesPerCell + best_index;
        float boxConfidence = l.output[confIndex];
        //*(l.cost) -= l.noobject_scale * pow(noObjectTarget - boxConfidence, 2);  // Box Confidence Term.
        //*(l.cost) += l.object_scale   * pow(trueObjectTarget - boxConfidence, 2);// Box Confidence Term.
        avgBoxConfidenceScore += boxConfidence;
        l.delta[confIndex] = l.object_scale * (trueObjectTarget - boxConfidence);// Box Confidence Term.

        if(l.rescore)
        {
          l.delta[confIndex] = l.object_scale * (iou - boxConfidence);
        }

        // Box terms.
        int i;
        for(i = 0; i < l.coords; ++i)
        {
          l.delta[box_index+i] = l.coord_scale*(state.truth[tbox_index + i] - l.output[box_index + i]);
        }

        if(l.sqrt)
        {
          l.delta[box_index+2] = l.coord_scale*(sqrt(state.truth[tbox_index + 2]) - l.output[box_index + 2]);
          l.delta[box_index+3] = l.coord_scale*(sqrt(state.truth[tbox_index + 3]) - l.output[box_index + 3]);
        }

        // Shape terms, count the number of pixels in the shape mask that correspond to the target, given some threshold.
        float shapeSqErr = 0.0f;
        const float threshold = 0.5f;
        int intersectionCount = 0;
        int unionCount = 0;
        //const float shape_scale = 0.1f;
        for(i = l.coords; i < (l.coords + l.shapeparams); ++i)
        {
          float truth = state.truth[tbox_index + i];
          float output = l.output[box_index + i];

          l.delta[box_index+i] = l.shape_scale*(truth - output);

          if((truth > threshold) || (output > threshold)) ++unionCount;
          if((truth > threshold) && (output > threshold)) ++intersectionCount;

          //*(l.cost) += pow(l.delta[box_index+i],2);
          avg_shape_sq_err += pow(l.delta[box_index+i], 2);
        }

        float shapeScore = (float)intersectionCount/(float)unionCount;
        avg_shape_iou += shapeScore;

        //const float iouTarget = 1.0f;
        //*(l.cost) += pow(iouTarget - iou, 2); // This is not the actual cost that is being minimized.
        avg_iou += iou;

        ++count; // Increment the object count.
      }
    }

    *(l.cost) = pow(mag_array(l.delta, l.outputs * batchSize), 2);

#define DEBUG_DETECTION_LAYER
#ifdef DEBUG_DETECTION_LAYER
  static int isInitialised = 0;

  char buffer[1024];
  if(l.shapeparams > 0)
  {
    sprintf(buffer, "AvgDetIOU: %f, AvgShapeDetIOU: %f, AvgShapeSqErr: %f, AvgTrueClassPredProb: %f, AvgAllClassPredProb: %f, AvgTrueBoxConf: %f, AvgAnyBoxConf: %f, ObjectCount: %d\n",
                   avg_iou/count, avg_shape_iou/count, avg_shape_sq_err/count, avgPrecitedProbTrueCategories/count, avgPredictedProbAllCategories/(count*l.classes), avgBoxConfidenceScore/count, avgAnyBoxConfidenceScore/(batchSize*cellCount*boxesPerCell), count);
  }
  else
  {
    sprintf(buffer, "AvgDetIOU: %f, AvgTrueClassPredProb: %f, AvgAllClassPredProb: %f, AvgTrueBoxConf: %f, AvgAnyBoxConf: %f, ObjectCount: %d\n",
                   avg_iou/count, avgPrecitedProbTrueCategories/count, avgPredictedProbAllCategories/(count*l.classes), avgBoxConfidenceScore/count, avgAnyBoxConfidenceScore/(batchSize*cellCount*boxesPerCell), count);
  }

  static char logFile[128];
  if(!isInitialised)
  {
    time_t currentTime = time(NULL);
    sprintf(logFile, "/tmp/debug-detection-layer-%ld.log", currentTime);
    isInitialised = 1;
  }
  FILE *f = fopen(logFile, "a");
  if(f == NULL)
  {
    printf("Cannot write to %s\n", logFile);
  }
  else
  {
    fprintf(f, "%s", buffer);
  }
  fclose(f);

  printf("%s", buffer);
#endif
#undef DEBUG_DETECTION_LAYER
  }
}

void backward_detection_layer(const detection_layer l, network_state state)
{
    axpy_cpu(l.batch*l.inputs, 1, l.delta, 1, state.delta, 1);
}

#ifdef WITH_CUDA

void forward_detection_layer_gpu(const detection_layer l, network_state state)
{
    if(!state.train){
        copy_ongpu(l.batch*l.inputs, state.input, 1, l.output_gpu, 1);
        return;
    }

    float *in_cpu = calloc(l.batch*l.inputs, sizeof(float));
    float *truth_cpu = 0;
    if(state.truth){
        int num_truth = l.batch*l.side*l.side*(1+l.coords+l.shapeparams+l.classes);
        truth_cpu = calloc(num_truth, sizeof(float));
        cuda_pull_array(state.truth, truth_cpu, num_truth);
    }
    cuda_pull_array(state.input, in_cpu, l.batch*l.inputs);
    network_state cpu_state = state;
    cpu_state.train = state.train;
    cpu_state.truth = truth_cpu;
    cpu_state.input = in_cpu;
    forward_detection_layer(l, cpu_state);
    cuda_push_array(l.output_gpu, l.output, l.batch*l.outputs);
    cuda_push_array(l.delta_gpu, l.delta, l.batch*l.inputs);
    free(cpu_state.input);
    if(cpu_state.truth) free(cpu_state.truth);
}

void backward_detection_layer_gpu(detection_layer l, network_state state)
{
    axpy_ongpu(l.batch*l.inputs, 1, l.delta_gpu, 1, state.delta, 1);
    //copy_ongpu(l.batch*l.inputs, l.delta_gpu, 1, state.delta, 1);
}
#endif

