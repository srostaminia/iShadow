%%


clear;
close all;

addpath('/Users/ytun/iShadow/evaluation/Error_graphs')
subList={'akshaya'};%,'mike'%{'malai'};%'addison','duncan',
rootDir='/Users/ytun/iShadow/algorithms/cider/data/';
cd ~
cd (rootDir)

irbSub='pupil_percent';

lighting='calib_pupil';


% CHANGE HERE:
nLambda=10;

objForTitle='pupil location';%'pupil area';
ylabelunit='error (pixel)';

dataObj='target';%'radius diff'; %radius diff
 object='pupil';
 modelName='ann'; % 'ANN' %'both'
 
 subName=subList{1}
fileToSave=strcat('percentile_',subName);%'train_inAndOut,test_out';

irbFolderName=strcat('awesomeness_irb_pupil');

subDir= fullfile(rootDir,irbFolderName);
 titleTxt={strcat(subName,': Comparison of with and without cropping 5 percentile')};
 %titleTxt={'Error of ANN pupil location estimation(addison-outmodeLong-and-benchmark)','training set=outmode+inmode'};
extra1='without percentile technique';
fileToOpen=strcat(modelName,'_','rep%d.mat');
legendToPut='';

figure;
graph_error_model_rad(lighting,modelName,subList,subDir,objForTitle,ylabelunit,dataObj,titleTxt,fileToOpen,fileToSave,legendToPut);
%[errStEachSub,meanErrAllSub,stdErrAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model_rad(lighting,modelName,subList,subDir,objForTitle,ylabelunit,dataObj,titleTxt,fileToOpen,fileToSave,legendToPut);
hold on;

irbFolderName=strcat('awesomeness_irb_',irbSub);

subDir= fullfile(rootDir,irbFolderName);
extra2='with percentile technique';
fileToOpen=strcat(modelName,'_','percent','rep%d.mat');
legendToPut={extra1,extra2};

graph_error_model_rad(lighting,modelName,subList,subDir,objForTitle,ylabelunit,dataObj,titleTxt,fileToOpen,fileToSave,legendToPut);
hold off;


%%
% % CHANGE HERE:
%  modelName='cider'; % 'ANN' %'both'
% [ciderErrStEachSub,ciderMeanErrorMat,ciderStdErrorMat,lineMeanErrorMat,lineStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit,dataObj);



% %%
% %size(ciderMeanErrorMat)
% %size(lineMeanErrorMat)
% %%
% error2modelMat=[annMeanErrorMat' ciderMeanErrorMat' lineMeanErrorMat'];
% std2modelMat=[annStdErrorMat' ciderStdErrorMat' lineStdErrorMat'];
% 
% %compare_barGraph(error2modelMat,subList,nLambda)
% 
% nUsers=length(subList);
% 
% figure;
% bar(1:1:nLambda,error2modelMat);%,std2modelMat);
% 
% %errorb([(1:1:nLambda)' (1:1:nLambda)' (1:1:nLambda)'],error2modelMat);%,std2modelMat);
% xlim([0 nLambda+1])
% ylabel('error %');
% xlabel('ANN size (incrementing)');
% 
% legend('ANN','CIDER','LINE');  %%%%%CHANGE HERE!!
% suptitle('Error of pupil center prediction for ANN vs. CIDER');
% 
% % %%
% % diffErr2Model=(ciderMeanErrorMat-annMeanErrorMat);
% % 
% % diffErr2Model
% % 
% % figure;
% % bar(1:1:nLambda,diffErr2Model);
% % ylabel('error');
% % xlabel('ANN size (incrementing)');
% % suptitle({'Error of pupil center prediction for ANN vs. CIDER','negative error- ANN error> CIDER error','positive error- ANN error< CIDER error','zero error- ANN error= CIDER error'});
% % 
