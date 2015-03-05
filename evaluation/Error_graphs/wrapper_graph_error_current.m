clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison','duncan','kaituo','malai','mike','shuo'};
folderName='awesomeness_yamin_cider_test';%'awesomeness_irb_radius_target_mean';%%irb_pupil';
rootDir=fullfile('~/iShadow/algorithms/cider/data/new',folderName);
addpath('~/iShadow/algorithms/cider/');

lightingSubFolder='_calib_pupil';%
uni='uniquefy_0';
objToPlotList={'pupilLoc-pixel','pupilLoc-deg','area','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
titleObjToPlotList={'pupil center error (pixels)','pupil center error (degrees)','pupil area percentage error','pupil radius absolute error'}; %pupilLoc_deg,pupilLoc_pixel,area,radius

unitList={'#pixels','degree','%','#pixels'};
saveOn=0;

graphDir='/Users/ytun/Google Drive/IMPORTANT_VISION/Graphs';
subpathToSave=fullfile(graphDir,folderName);
mkdir(subpathToSave);

subModelName='';%'rerun2_testind_';

linestylelist={'-','--','o-','*-'};

nUsers=length(subList);

iGaze_err_deg=(91*5+10*8+15)/100; %page 9


% CHANGE HERE:
modelName='cider'; % 'ANN' %'both'

%Compare line vs. ann (only line frames)
%or compare ann vs. cider (all frames)
compareLineList={0 1};

modelvsStrList={'ANN vs. CIDER','ANN vs. Line'};

for mdInd=1:length(modelvsStrList)
    
    for objInd=3%1:length(objToPlotList)
        
        if compareLineList{mdInd}
            pathToSave=fullfile(subpathToSave,objToPlotList{objInd},'lineCmpr');
        else
            pathToSave=fullfile(subpathToSave,objToPlotList{objInd},'AllFramesCmpr');
        end
        
        mkdir(pathToSave);
        
        
        %Error graphs for all subjects: ANN and Cider OVERLAP
        for indFile=1
            figure;
            
            linestyle=linestylelist{indFile};
            nLambda=10;
            modelName='ann'; % 'ANN' %'both'
            [annErrStEachSub,~,annMeanErrorAllSub,annStdErrorAllSub,annIndPerAllSub]=graph_error_model_current(indFile,linestylelist{1},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName,compareLineList{mdInd});
            
            hold on;
            
            % CHANGE HERE:
            modelName='cider'; % 'ANN' %'both'
            [ciderErrStEachSub,err_line_StEachSub,ciderMeanErrorAllSub,ciderStdErrorAllSub,ciderIndPerAllSub,lineMeanErrorAllSub,lineStdErrorAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(indFile,linestylelist{2},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName,compareLineList{mdInd});
            
            
            
        end
        
        hold off;
        limsy=get(gca,'YLim');
        set(gca,'Ylim',[0 limsy(2)]);
        %ylim([0, 2.9]) %10.1
        xlim([0 nLambda+1])
        ylabel(sprintf('Error (%s)',unitList{objInd}));
        xlabel('ANN size (Percent active pixels)');
        legend([strcat('ann','-',subList),strcat('cider','-',subList)]);
        suptitle(sprintf('%s: %s error for %i different users',modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers));
        
        %suptitle(sprintf('%s: Error for %i different users',modelName,nUsers));
        plotFileName=strcat(objToPlotList{objInd},'_indivUsers_ciderANN_combo');
        
        plotFormatQ_final(pathToSave,plotFileName,saveOn);
        
        figure;
        
        %Error graphs for all subjects: ANN and Cider SUBPLOT
        for i=1:nUsers %nUsers x nLambda: 6x10
            subplot(3,3,i);
            subName=subList{i};
            
           
            errorbar(1:1:nLambda,annErrStEachSub.mean.(uni).(subName),annErrStEachSub.std.(uni).(subName),strcat(linestyle,'r'),'markers',10);
            hold on;
            errorbar(1:1:nLambda,ciderErrStEachSub.mean.(uni).(subName),ciderErrStEachSub.std.(uni).(subName),strcat(linestyle,'b'),'markers',10);
            errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName),strcat(linestyle,'m'),'markers',10);
            
            limsy=get(gca,'YLim');
            set(gca,'Ylim',[0 limsy(2)]);
            xlim([0 nLambda+1])
            ylabel(sprintf('Error (%s)',unitList{objInd}));
            xlabel('ANN size (Percent active pixels)');
            title(subName);
            
        end
        
        hold off;
        
        
        legend({'ann','cider','line'});
        suptitle(sprintf('%s: %s error for %i different users',modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers));
        
        plotFileName=strcat(objToPlotList{objInd},'_indivUsers_ciderANNLine_subplot');
        origin=pwd;
        
        cd ~
        cd(pathToSave);
        
        saveas(gcf, plotFileName,'png')
        cd (origin);
        
        
        %'ann-5 percentile','cider-5 percentile','line-5 percentile','ann-0.05 percentile','cider-0.05 percentile','line-0.05 percentile','ann-2 percentile','cider-2 percentile','line-2 percentile','ann-mean','cider-mean','line-mean'});
        
        
        % bar plot of all users
        
        %if strcmp(objToPlotList{objInd},'pupilLoc-deg')
        %     error2modelMat=[[annMeanErrorAllSub 0]' [ciderMeanErrorAllSub 0]' [iGaze_err_deg 0 ]'];
        %     std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]' [ciderStdErrorAllSub 0]' [iGaze_err_deg 0]'];
        %
        
        if compareLineList{mdInd}
            error2modelMat=[[annMeanErrorAllSub 0]' [lineMeanErrorAllSub 0]' ];
            std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]' [lineStdErrorAllSub 0]' ];
            legendlist={'ANN error','Line model error'};
            
        else
            error2modelMat=[[annMeanErrorAllSub 0]' [ciderMeanErrorAllSub 0]' ];
            std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]' [ciderStdErrorAllSub 0]' ];
            legendlist={'ANN error','CIDER error'};
            
        end
        
        
        % error2modelMat=[[annMeanErrorAllSub 0]' [ciderMeanErrorAllSub 0]' [lineMeanErrorAllSub 0]'];
        % std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]' [ciderStdErrorAllSub 0]' [lineStdErrorAllSub 0]' ];
        
        xticklist=cell(1,length(annIndPerAllSub)+1);
        
        for i=1:length(annIndPerAllSub)
            xticklist{i}=sigfig(annIndPerAllSub(i),2);
        end
        xticklist{i+1}='100';
        
        %ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];
        
        %compare_barGraph(error2modelMat,subList,nLambda)
        
        nUsers=length(subList);
        
        figure;
        upperErExist=0;
        SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist,upperErExist);
        ylabel(sprintf('Average Error (%s)',unitList{objInd}));
        xlabel('ANN Size (Percent active pixels)');
        
        
        suptitle(sprintf('%s: Average %s for %i different users',modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers));
        
        %suptitle('Error of pupil center prediction of ANN vs. CIDER');
        %xlim([0 11.2])
        if strcmp(unitList{objInd},'%')
            ylim([0 100])
        else
            limsy=get(gca,'YLim');
            set(gca,'Ylim',[0 limsy(2)*1.2]);
        end
        
        plotFileName=strcat(objToPlotList{objInd},'_average_ciderANN_barGraph');
        plotFormatQ_final(pathToSave,plotFileName,saveOn);
    end
    
end
%% ANN used

error2modelMat=[[meanAnnUsedForAllSub 0]' [meanLineUsedForAllSub 0]'];
std2modelMat=1.96*(1/sqrt(nUsers))*[[stdAnnUsedForAllSub 0]' [stdLineUsedForAllSub 0]' ]; %1.96*

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
upperErExist=1;
SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist,upperErExist);
ylim([0 120])
ylabel('Percentage frequency of model usage (%)');
xlabel('ANN Size (Percent active pixels)');
suptitle('Percentage frequency of ANN model and line model usage for CIDER');

%suptitle('Error of pupil center prediction of ANN vs. CIDER');
%xlim([0 11.2])


plotFileName='frequency_ciderLine_barGraph';
plotFormatQ_final(fullfile(pathToSave,'../..'),plotFileName,saveOn);
