clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evaluation/Error_graphs')
subList={'malai','mike','shuo','kaituo'};%,'addison','duncan'};%'addison',,'duncan'
folderName='awesomeness_irb_radius_target_DARKcrossval';%_outdoorFiltMedDia100Train';%'awesomeness_irb_radius_target_outdoorFiltMedBox100Train';%'awesomeness_irb_radius_target_dark80%train';%'awesomeness_irb_target_outdoor';%'awesomeness_irb_radius_target_mean';%%irb_pupil';
lighting='dark';

%%
rootDir=fullfile('/Users/ytun/Documents/ISHADOW_DATA/new',folderName);
addpath('~/iShadow/algorithms/cider/');
lightingSubFolder=strcat('_',lighting,'_pupil');%_ohsocoldFiltMedDia_pupil';%'_calib_pupil';%'_ohsocoldFiltMedBox_pupil';%'_calib_pupil';%;;%
uni='uniquefy_0';
objToPlotList={'pupilcentPix','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
titleObjToPlotList={'pupil center error (pixels)','pupil radius error (pixels)'}; %pupilLoc_deg,pupilLoc_pixel,area,radius

unitList={'#pixels','#pixels'};%,'degree','%',
plotSaveOn=0;
dataSaveOn=1;

graphDir='/Users/ytun/Google Drive/IMPORTANT_VISION/Graphs';
subpathToSave=fullfile(graphDir,folderName);

if plotSaveOn
    mkdir(subpathToSave);
end

% if saveOn
% mkdir(subpathToSave);
% end

subModelNameList={'','','idealLINE_'};%'dark20Test_';%darkTest_';%'rerun2_testind_';
linestylelist={'-','--','o-','*-'};

nUsers=length(subList);

%iGaze_err_deg=(91*5+10*8+11)/100; %page 9

% CHANGE HERE:
%modelName='ann'; % 'ANN' %'both'

%Compare line vs. ann (only line frames)
%or compare ann vs. cider (all frames)

subFolderName_list={'AllFramesCmpr','lineCmpr','IdealLine'};
stage_list={'aggregate','switching','ideal'};%,'IdealLine'};

modelvsStrList={'ANN vs. CIDER','ANN vs. Cross','Ideal Cross'};

nLambda=10;

checkpass=length(subModelNameList)==length(subFolderName_list) && length(subFolderName_list)==length(modelvsStrList) && length(modelvsStrList)==length(stage_list);

% % data to save
% ideal=struct;
% switching=struct;
errData=struct;
% %

if ~checkpass
    fprintf('ERROR: check subModelNameList,subFolderName_list,modelvsStrList in settings');
else
    for mdInd=1:length(modelvsStrList)
        
        subModelName=subModelNameList{mdInd};

        compareLine=strcmp(modelvsStrList{mdInd},'ANN vs. Cross');% 2};

        for objInd=1:length(objToPlotList)
            
            %
            %
            %         if compareLine==1
            %             pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,'lineCmpr'));
            %             fileSub=strcat('lineCmpr_',subModelName);
            %
            %         elseif compareLine==0
            %             pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,'AllFramesCmpr'));
            %             fileSub=strcat('AllFramesCmpr_',subModelName);
            %
            %         elseif compareLine==2
            pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,subFolderName_list{mdInd}));
            fileSub=strcat(subFolderName_list{mdInd},'_',subModelName);
            
            %        % else
            %             fprintf('ERROR: fix compareLineList items.');
            %             return;
            %         end
            
            if plotSaveOn
                mkdir(pathToSave);
            end
            
            %Error graphs for all subjects: ANN and Cider OVERLAP
            for indFile=1 %always 1
                figure;
                
                linestyle=linestylelist{indFile};
                %nLambda=10;
                
                modelName='ann'; % 'ANN' %'both'
                
                if strcmp(modelvsStrList{mdInd},'Ideal Cross')
                    subModelName_ann='';
                else
                    subModelName_ann=subModelName;
                end
                

                [annErrStEachSub,~,annMeanErrorAllSub,annStdErrorAllSub,annIndPerAllSub]=graph_error_model_current_meanOfmean(indFile,linestylelist{1},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName_ann,compareLine,lighting);
                
                if dataSaveOn 
                    errData.(stage_list{mdInd}).(modelName).(objToPlotList{objInd}).err=annMeanErrorAllSub;
                    errData.(stage_list{mdInd}).(modelName).(objToPlotList{objInd}).std=1.96*(1/sqrt(nUsers))*annStdErrorAllSub;
                   errData.(stage_list{mdInd}).(modelName).indper=annIndPerAllSub;
                   
                end
                %nLambda=9;
                hold on;
                
                % CHANGE HERE:
                modelName='cider'; % 'ANN' %'both'
                [ciderErrStEachSub,err_line_StEachSub,ciderMeanErrorAllSub,ciderStdErrorAllSub,ciderIndPerAllSub,lineMeanErrorAllSub,lineStdErrorAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current_meanOfmean(indFile,linestylelist{2},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName,compareLine,lighting);
               
                if dataSaveOn
                    errData.(stage_list{mdInd}).(modelName).(objToPlotList{objInd}).err=lineMeanErrorAllSub;
                    errData.(stage_list{mdInd}).(modelName).(objToPlotList{objInd}).std=1.96*(1/sqrt(nUsers))*lineStdErrorAllSub;
                    errData.(stage_list{mdInd}).(modelName).freqANNUsed=meanLineUsedForAllSub;
              
                end
                
            end
            
            hold off;
            limsy=get(gca,'YLim');
            set(gca,'Ylim',[0 limsy(2)]);
            %ylim([0, 2.9]) %10.1
            xlim([0 nLambda+1])
            ylabel(sprintf('Error (%s)',unitList{objInd}));
            xlabel('ANN size (Percent active pixels)');
            legend([strcat('ann','-',subList),strcat('cider','-',subList)]);
            suptitle(sprintf('%s- %s: %s error for %i different users',stage_list{mdInd},modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers));
            
            %suptitle(sprintf('%s: Error for %i different users',modelName,nUsers));
            
            plotFileName=strcat(fileSub,objToPlotList{objInd},'_indivUsers_ciderANN_combo');
            plotMode='';
            plotFormatQ_final(pathToSave,plotFileName,plotSaveOn,plotMode);
            
            figure;
            
            %Error graphs for all subjects: ANN and Cider SUBPLOT
            for i=1:nUsers %nUsers x nLambda: 6x10
                subplot(3,3,i);
                subName=subList{i};
                
                errorbar(1:nLambda,annErrStEachSub.mean.(uni).(subName),annErrStEachSub.std.(uni).(subName),strcat(linestyle,'r'),'markers',10);
                hold on;
                
                %errorbar(1:nLambda,ciderErrStEachSub.mean.(uni).(subName),ciderErrStEachSub.std.(uni).(subName),strcat(linestyle,'b'),'markers',10);
                %errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName),strcat(linestyle,'m'),'markers',10);
                hold off;
                
                limsy=get(gca,'YLim');
                set(gca,'Ylim',[0 limsy(2)]);
                xlim([0 nLambda+1])
                ylabel(sprintf('Error (%s)',unitList{objInd}));
                xlabel('ANN size (Percent active pixels)');
                title(subName);
                
            end
            
            
            
            legend({'ann','cider','line'});
            suptitle(sprintf('%s-%s: %s error for %i different users',stage_list{mdInd},modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers));
            
            plotFileName=strcat(fileSub,objToPlotList{objInd},'_indivUsers_ciderANNLine_subplot');
            origin=pwd;
            
            if plotSaveOn
            cd ~
            cd(pathToSave);
            
            saveas(gcf, plotFileName,'png')
            cd (origin);
            end
            
            %'ann-5 percentile','cider-5 percentile','line-5 percentile','ann-0.05 percentile','cider-0.05 percentile','line-0.05 percentile','ann-2 percentile','cider-2 percentile','line-2 percentile','ann-mean','cider-mean','line-mean'});
            
            
            % bar plot of all users
            
            %if strcmp(objToPlotList{objInd},'pupilLoc-deg')
            %     error2modelMat=[[annMeanErrorAllSub 0]' [ciderMeanErrorAllSub 0]' [iGaze_err_deg 0 ]'];
            %     std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]' [ciderStdErrorAllSub 0]' [iGaze_err_deg 0]'];
            %
            
            if compareLine
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
            SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist);%,upperErExist);
            ylabel(sprintf('Average Error (%s)',unitList{objInd}));
            xlabel('ANN Size (Percent active pixels)');
            title(sprintf('%s-%s: Average %s for %i different users',stage_list{mdInd},modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers),'FontSize', 11);
            
            %suptitle('Error of pupil center prediction of ANN vs. CIDER');
            %xlim([0 11.2])
            if strcmp(unitList{objInd},'%')
                ylim([0 100])
            else
                limsy=get(gca,'YLim');
                set(gca,'Ylim',[0 limsy(2)*1.2]);
            end
            
            plotFileName=strcat(fileSub,objToPlotList{objInd},'_average_ciderANN_barGraph');
            plotMode='';
            plotFormatQ_final(pathToSave,plotFileName,plotSaveOn,plotMode);
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
            SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist);%,upperErExist);
            ylim([0 120])
            ylabel('Percentage frequency of model usage (%)');
            xlabel('ANN Size (Percent active pixels)');
            title(sprintf('%s- Percentage frequency of ANN model and line model usage for CIDER',stage_list{mdInd}),'FontSize', 11);
            
            %suptitle('Error of pupil center prediction of ANN vs. CIDER');
            %xlim([0 11.2])
            
            
            plotFileName='frequency_ciderLine_barGraph';
            plotMode='';
            plotFormatQ_final(fullfile(pathToSave),plotFileName,plotSaveOn,plotMode);
        
    end
    
    if dataSaveOn
        dir=fullfile('/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err');
        save(fullfile(dir,strcat(lighting,'_error_pplot.mat')),'errData');%,%'lineUsed_Lambdas_mean');
    end

end



                
%% ANN used

