clear;
close all;

addpath('~/iShadow/evaluation')
subList={'addison','duncan','mike'};%,'mike'%{'malai'};%'addison','duncan',
rootDir='~/iShadow/algorithms/cider/data/';

cd ~
cd (rootDir)

% CHANGE HERE:
nLambda=10;

objForTitle='predicted pupil area';
ylabelunit='error %';

 object='radius_target';
 modelName='ann'; % 'ANN' %'both'
 [annErrStEachSub,annMeanErrorMat,annStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit);

% % CHANGE HERE:
 modelName='cider'; % 'ANN' %'both'
[ciderErrStEachSub,ciderMeanErrorMat,ciderStdErrorMat,lineMeanErrorMat,lineStdErrorMat]=graph_error_model_rad(modelName,subList,rootDir,object,objForTitle,ylabelunit);



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
ylabel('error');
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
