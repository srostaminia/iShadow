clear;
close all;

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison'};%,'mike'%{'malai'};%'addison','duncan',
rootDir='~/iShadow/algorithms/cider/data/';

cd ~
cd (rootDir)

% CHANGE HERE:
nLambda=10;

objForTitle='pupil radius';%'pupil area';
ylabelunit='error (pixel)';

dataObj='target';%'radius diff'; %radius diff
 object='pupil';
 modelName='ann'; % 'ANN' %'both'
 %lighting='outdoors';
 lightingL={'outdoors','calib'};
 %title='outdoor data: (ANN predicted radius- ground truth average of ellipse axis lengths)';
 %title='first 600 frames: duncan-dark ran on ANN-calib-based'%;'frame 1: ANN RawOutput: duncan_outdoor on ANN (duncan_cali-based)';
 %title='frame 1:pushed left by 4 pixels: 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';%'frame 1 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';
 %title='frame 1087: indoor-dark -Feb 13 normalized with duncan-dark on ANN-calib-based';
 %title='all frames: no blink- raw- outdoors prediction Feb 13- retesting';
 title='Error of pupil location estimation- outdoor MALAI data tested on ANN (MALAI-calib-based)';
 %[annErrStEachSub,annMeanErrorMat,annStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);

 for a=1:2
     lighting=lightingL{a};

graph_error_model_rad(a,lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);
  %[errStEachSub,meanErrAllSub,stdErrAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model_rad(a,lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);

 end
hold off;
 ylabel('error(pixels)');
 xlabel('ANN size (incrementing)');
suptitle(strcat('pupil location error- ',subList{1}));
legend(lightingL);
% 

% % CHANGE HERE:
%  modelName='cider'; % 'ANN' %'both'
% [ciderErrStEachSub,ciderMeanErrorMat,ciderStdErrorMat,lineMeanErrorMat,lineStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);

