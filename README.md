## Straight To Shapes: Real-Time Detection of Encoded Shapes

<p align="center">
  <img src="visual_pipeline.png" width="1000">
</p>

*Project page:* [http://www.robots.ox.ac.uk/~tvg/projects/StraightToShapes](http://www.robots.ox.ac.uk/~tvg/projects/StraightToShapes/index.php)

**Abstract** :This work accomplishes direct regression to objects' shapes in addition to their bounding boxes and categories by introducing a compact and decodable shape embedding space using a denoising autoencoder.
A deep convolutional network is trained to regress to the low dimensional shape vectors which are then mapped to shape masks using the decoder half of the autoencoder.
Our end-to-end network qualifies as the first real-time instance segmentation pipeline running at ~35FPS while yielding promising results at the task.
Proposed top-down regression to object shape masks through a semantically defined shape space allows the network to generalize to unseen categories at test time. We call this zero-shot segmentation and evaluate the performance of our model at the task to establish a baseline for future research to be measured against.

----

### Results


- **Instance segmentation**
##### 1. Convergence plots of mAP scores @ 0.5 IoU 
mAP estimates over 2000 randomly selected train and val images from SBD dataset as network architectures minimize proxy L2-regression loss over output set of {shape params, bounding-box params, class-probabilities}.<br/>
[BN: using Batch normalization, DA: using stronger Data augmentation].
<img src=mAP.png width='700'>

##### 2. Quantitative results presenting mAP scores at different IoUs. 
 
| Archi.     | Shape space |mAP@0.5|mAP@0.7|mAP vol| Runtime (ms)| 
| -----      |------------ |:-------:|:-----------:|:------:|:------:| 
| **Ours**   |             |         |             |        |      |
| YOLO       | Binary mask | 32.3    | 12.0        | 28.6   | 26.3\* |
| YOLO       | Radial      | 30.0    | 6.5         | 29.0   | 27.1\* |
| YOLO       | Embedding (50) | 32.6 | 14.8        | 28.9   | 30.5\* |
| YOLO       | Embedding (20) | 34.6 | 15.0        | 31.5   | 28.0\* |
| YOLO-BN    | Embedding (20) | 38.6 | 17.4        | 34.3   | 28.0\* |
| YOLO-BN-DA | Embedding (20) | 42.3 | 20.8        | 36.9   | 28.0\* |
| **Others** |                |      |             |        |      |
| [SDS](https://arxiv.org/pdf/1407.1808.pdf) | -   | 49.7   | - | 41.4 | 48K |
| [MNC](https://arxiv.org/pdf/1512.04412.pdf) | -  | 63.5   | 41.5 | - | 360 |

\* These present the computational time of the entire application (including display and wait time for the user) not just the feed-forward prediction.

#### 3. Qualitative results
(a) Correct predictions using downsampled binary masks [YOLO-binarymask].<br/>
(b) Correct predictions using 20D learnt shape encodings [YOLO-embedding20]. In column-3, the horns of the cow are missed and the human shape mask gets elongated due to an incorrect bounding box prediction.<br/>
(c) Missed detections using the 20D shape encodings [YOLO-embedding20]. The network misses out or false fires on small objects in column-2, the dogs in the images are falsely categorised as cats, and the sofa incorrectly includes the nearby dining table.<br/>

<img src=instancesegmentation.png width='700'>



- **Zero-shot segmentation**

##### 1. Quantitative results
 
| Archi.| Shape space         |mAP@0.5 (all)|mAP@0.7 (large)|mAP vol (large)|
| ------|---------------------|:-----------:|:-------------:|:-------------:|
| YOLO   | Embedding (20)     | 34.6        | 15.0          | 31.5          |
| YOLO-BN| Embedding (20)     |             |               |               |
| YOLO-BN-DA | Embedding (20) |             |               |               |

#### 2. Qualitative results
A comparison between the state-of-the-art (a) [semantic segmentation](https://arxiv.org/pdf/1511.08119.pdf), (b) [instance segmentation](https://arxiv.org/abs/1609.02583), and (c) our shape detection results, on images from YouTube videos of animals that are not present in the PASCAL training set using YOLO-embedding20.<br/>
In the first two rows, instance segmentation predicts that the legs of the tiger are human. Our method is more consistent over the tiger images taken from the same video.<br/>
In the lower rows, the instance segmentation approach of (b) fails to predict any segments, whilst our method predicts class 'dog' for the tiger, hedgehog, baby elephant and bear, and class 'horse' for the large elephant.

<img src=ZSLcomparisonToStateOfTheArt.png width='700'>


### Acknowledgements
This version of the *StraightToShapes* concept was implemented by [Saumya Jetley](http://saumya-jetley.github.io/) and [Michael Sapienza](http://sites.google.com/site/mikesapi) and [Stuart Golodetz](http://research.gxstudios.net/), under the supervision of [Professor Philip Torr](http://www.robots.ox.ac.uk/~tvg). Additional experiments with batch normalization, data augmentation have been contributed by [Laurynas Miksys](https://www.linkedin.com/in/laurynasmiksys/).

It is built on top of [Darknet](https://github.com/pjreddie/darknet), an open-source neural network framework developed by [Joseph Redmon](http://pjreddie.com/).

If you build on this framework for your research, please consider citing the original research paper:
```
@article{JetleySapienza2016,
  author    = {Saumya Jetley and
               Michael Sapienza and
               Stuart Golodetz and
               Philip H. S. Torr},
  title     = {Straight to Shapes: Real-time Detection of Encoded Shapes},
  journal   = {CoRR},
  volume    = {abs/1611.07932},
  year      = {2016},
  url       = {http://arxiv.org/abs/1611.07932},
}
```
------------------
### Installation Guide
To install the software, follow the instructions provided [HERE](INSTALL.md).

------------------
### License
This work is protected by a [license](LICENSE.md) agreement.


