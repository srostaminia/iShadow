% clear;
% close all;

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
 lightingL={'test data =outdoor','test data =outmode(same as train)'};
 %title='outdoor data: (ANN predicted radius- ground truth average of ellipse axis lengths)';
 %title='first 600 frames: duncan-dark ran on ANN-calib-based'%;'frame 1: ANN RawOutput: duncan_outdoor on ANN (duncan_cali-based)';
 %title='frame 1:pushed left by 4 pixels: 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';%'frame 1 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';
 %title='frame 1087: indoor-dark -Feb 13 normalized with duncan-dark on ANN-calib-based';
 %title='all frames: no blink- raw- outdoors prediction Feb 13- retesting';
 title='Error of pupil location estimation- outdoor MALAI data tested on ANN (MALAI-calib-based)';
 %[annErrStEachSub,annMeanErrorMat,annStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);

% fileNameList={strcat(modelName,'_rep',num2str(m),'.mat'),strcat(modelName,'_rep','_outdoor_test_Feb13',num2str(m),'.mat')};
 lighting='outmode_long_eye';
 %for a=1:2
     %fileName=fileNameList{a};
     %lighting=lightingL{a};
     %rep_dir=fullfile(rootDir,strcat('awesomeness_irb_',object),strcat(subName,'_',lighting),uni,'results',lambdaFolder,fileName);
 
graph_error_model_rad(lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);
 % graph_error_model_rad(lighting,modelName,subList,rep_dir,dataObj);
hold on;
    %lighting=lightingL{a};
     %rep_dir=fullfile(rootDir,strcat('awesomeness_irb_',object),strcat(subName,'_',lighting),uni,'results',lambdaFolder,fileName);
graph_error_model_rad_mod(lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);
 % graph_error_model_rad(lighting,modelName,subList,rep_dir,dataObj);

 %[errStEachSub,meanErrAllSub,stdErrAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model_rad(a,lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);

 %end
hold off;
 ylabel('error(pixels)');
 xlabel('ANN size (incrementing)');
suptitle(strcat('pupil location error- ',subList{1}));
legend(lightingL);
% 

% % CHANGE HERE:
%  modelName='cider'; % 'ANN' %'both'
% [ciderErrStEachSub,ciderMeanErrorMat,ciderStdErrorMat,lineMeanErrorMat,lineStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);

