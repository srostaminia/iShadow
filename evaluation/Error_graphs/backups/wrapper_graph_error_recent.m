clear;
close all;

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison'};%,'mike'%{'malai'};%'addison','duncan',
rootDir='~/iShadow/algorithms/cider/data/';

cd ~
cd (rootDir)

% CHANGE HERE:
nLambda=10;

objForTitle='pupil';%'pupil area';
ylabelunit='error (pixel)';

dataObj='target';%'radius diff'; %radius diff
 object='pupil';
 modelName='ann'; % 'ANN' %'both'
 lighting='outmodeLong_and_benchmark';
 
 %title='outdoor data: (ANN predicted radius- ground truth average of ellipse axis lengths)';
 %title='first 600 frames: duncan-dark ran on ANN-calib-based'%;'frame 1: ANN RawOutput: duncan_outdoor on ANN (duncan_cali-based)';
 %title='frame 1:pushed left by 4 pixels: 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';%'frame 1 2nd-normalized dist with dark: duncan-outdoor ran on ANN-calib-based';
 %title='frame 1087: indoor-dark -Feb 13 normalized with duncan-dark on ANN-calib-based';
 %title='all frames: no blink- raw- outdoors prediction Feb 13- retesting';
 title='Error of pupil location estimation- Mixed of outmode+inmode(addison-outmodeLong-and-benchmark)';
 %[annErrStEachSub,annMeanErrorMat,annStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);
[errStEachSub,meanErrAllSub,stdErrAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model_rad_copy(lighting,modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj,title);

% % CHANGE HERE:
%  modelName='cider'; % 'ANN' %'both'
% [ciderErrStEachSub,ciderMeanErrorMat,ciderStdErrorMat,lineMeanErrorMat,lineStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);



%%
%size(ciderMeanErrorMat)
%size(lineMeanErrorMat)
%%
error2modelMat=[annMeanErrorMat' ciderMeanErrorMat' lineMeanErrorMat'];
std2modelMat=[annStdErrorMat' ciderStdErrorMat' lineStdErrorMat'];

%compare_barGraph(error2modelMat,subList,nLambda)

nUsers=length(subList);

figure;
bar(1:1:nLambda,error2modelMat);%,std2modelMat);

%errorb([(1:1:nLambda)' (1:1:nLambda)' (1:1:nLambda)'],error2modelMat);%,std2modelMat);
xlim([0 nLambda+1])
ylabel('error %');
xlabel('ANN size (incrementing)');

legend('ANN','CIDER','LINE');  %%%%%CHANGE HERE!!
suptitle('Error of pupil center prediction for ANN vs. CIDER');

% %%
% diffErr2Model=(ciderMeanErrorMat-annMeanErrorMat);
% 
% diffErr2Model
% 
% figure;
% bar(1:1:nLambda,diffErr2Model);
% ylabel('error');
% xlabel('ANN size (incrementing)');
% suptitle({'Error of pupil center prediction for ANN vs. CIDER','negative error- ANN error> CIDER error','positive error- ANN error< CIDER error','zero error- ANN error= CIDER error'});
% 
