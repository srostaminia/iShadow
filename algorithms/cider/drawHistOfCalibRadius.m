
close all;
clear;


labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';

subName='duncan';

subFolderName=strcat(subName,'_','calib','_raw_ellipse.mat');
out_data=load(fullfile(labelPupilDir,subFolderName));

major=out_data.bestFitsMat(:,3);
minor=out_data.bestFitsMat(:,4);

rad_pupil_ell=sqrt(minor.*major);

figure;
subplot(1,3,1);
histogram(rad_pupil_ell);
title('Calib Duncan');


labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';

subName='duncan';

subFolderName=strcat(subName,'_','dark','_raw_ellipse.mat');
out_data=load(fullfile(labelPupilDir,subFolderName));

major=out_data.bestFitsMat(:,3);
minor=out_data.bestFitsMat(:,4);

rad_pupil_ell=sqrt(minor.*major);

subplot(1,3,2);
histogram(rad_pupil_ell);
title('Dark Duncan');


labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';

subName='duncan';

subFolderName=strcat(subName,'_','outdoors','_raw_ellipse.mat');
out_data=load(fullfile(labelPupilDir,subFolderName));

major=out_data.bestFitsMat(:,3);
minor=out_data.bestFitsMat(:,4);

rad_pupil_ell=sqrt(minor.*major);

subplot(1,3,3);
histogram(rad_pupil_ell);
title('Outdoors Duncan');


suptitle('Histogram of ground truth radius of pupil-squareRoot(0.5*(majorAxis*minorAxis))');