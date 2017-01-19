function ap = evaluate_detection(resultsPath, resultsFilename, competitionCode, testset)

% resultsPath =
% '/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/results/detection';
% timeStamp = '20160729T144304';
% competitionCode = 'comp4';

% change this path if you install the VOC code elsewhere
addpath([cd '/VOCcode']);

% initialize VOC options
VOCinit;

VOCopts.detrespath= [resultsPath, '/%s_%s.txt'];
VOCopts.testset = testset;

resultsPath = [resultsPath '/', resultsFilename];

ap = zeros(1,VOCopts.nclasses);
% train and test detector for each class
parfor i=1:VOCopts.nclasses
    [~,~,ap(i)]=VOCevaldet(VOCopts,competitionCode,VOCopts.classes{i},false);  % compute and display PR
end

fileID = fopen(resultsPath,'w');
for i=1:VOCopts.nclasses
    fprintf(fileID, '%s %0.10f\n', VOCopts.classes{i}, ap(i));
end
fclose(fileID);

end
