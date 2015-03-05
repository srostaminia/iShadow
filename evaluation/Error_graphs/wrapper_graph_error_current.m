clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison'};%{'yamin','addison','duncan','mike'};
folderName='awesomeness_irb_radius_target_circle';
rootDir=fullfile('~/iShadow/algorithms/cider/data',folderName);

lightingSubFolder='_calib_pupil';

objToPlot='radius'; %pupilLoc_deg,pupilLoc_pixel,area,radius

% CHANGE HERE:
nLambda=10;
modelName='ann'; % 'ANN' %'both'
[annErrStEachSub,annMeanErrorAllSub,annStdErrorAllSub,annIndPerAllSub]=graph_error_model_current(modelName,subList,rootDir,lightingSubFolder,objToPlot);

% CHANGE HERE:
modelName='cider'; % 'ANN' %'both'
[ciderErrStEachSub,ciderMeanErrorAllSub,ciderStdErrorAllSub,ciderIndPerAllSub,lineMeanErrorAllSub,lineStdErrorAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(modelName,subList,rootDir,lightingSubFolder,objToPlot);



%% bar plot
error2modelMat=[[annMeanErrorAllSub 0]' [ciderMeanErrorAllSub 0]' ];
std2modelMat=1.96*[[annStdErrorAllSub 0]' [ciderStdErrorAllSub 0]' ];

xticklist=cell(1,length(annIndPerAllSub)+1);

for i=1:length(annIndPerAllSub)
    xticklist{i}=sigfig(annIndPerAllSub(i),2);
end
 xticklist{i+1}='100';

%ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];

%compare_barGraph(error2modelMat,subList,nLambda)

nUsers=length(subList);

legendlist={'ANN error','CIDER error'};

figure;
SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist);
ylabel('Error (#pixels)');
xlabel('ANN Size (Percent Active Pixels)');
suptitle('Error of pupil size estimation of ANN vs. CIDER');

%suptitle('Error of pupil center prediction of ANN vs. CIDER');
%xlim([0 11.2])
limsy=get(gca,'YLim');
set(gca,'Ylim',[0 limsy(2)*1.5]);

plotFileName='radiusError';
pathToSave='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/Graphs';

plotFormatQ_final(pathToSave,plotFileName,1);


%% ANN used


error2modelMat=[[meanAnnUsedForAllSub 0]' [meanLineUsedForAllSub 0]'];
std2modelMat=1.96*[[stdAnnUsedForAllSub 0]' [stdLineUsedForAllSub 0]' ];

xticklist=cell(1,length(annIndPerAllSub)+1);

for i=1:length(annIndPerAllSub)
    xticklist{i}=sigfig(annIndPerAllSub(i),2);
end
 xticklist{i+1}='100';

%ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];

%compare_barGraph(error2modelMat,subList,nLambda)

nUsers=length(subList);

legendlist={'ANN frequency','Line Model frequency'};

figure;
SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist);
ylabel('Percentage frequency of model use');
xlabel('ANN Size (Percent Active Pixels)');
suptitle('Fraction of time each model runs ANN vs. CIDER');
ylim([0 100])

%suptitle('Error of pupil center prediction of ANN vs. CIDER');
%xlim([0 11.2])


plotFileName='frequency_model_used';
pathToSave='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/Graphs';

plotFormatQ_final(pathToSave,plotFileName,1);

