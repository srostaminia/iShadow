clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison'};%'addison',,'duncan'
folderName='awesomeness_irb_radius_target_outdoorRaw100Train';%_outdoorFiltMedDia100Train';%'awesomeness_irb_radius_target_outdoorFiltMedBox100Train';%'awesomeness_irb_radius_target_dark80%train';%'awesomeness_irb_target_outdoor';%'awesomeness_irb_radius_target_mean';%%irb_pupil';
lighting='ohsocold';

rootDir=fullfile('/Users/ytun/Documents/ISHADOW_DATA/new',folderName);
addpath('~/iShadow/algorithms/cider/');
lightingSubFolder=strcat('_',lighting,'_pupil');%_ohsocoldFiltMedDia_pupil';%'_calib_pupil';%'_ohsocoldFiltMedBox_pupil';%'_calib_pupil';%;;%
uni='uniquefy_0';
objToPlotList={'pupilcentPix','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
titleObjToPlotList={'pupil center error (pixels)','pupil radius error (pixels)'}; %pupilLoc_deg,pupilLoc_pixel,area,radius

unitList={'#pixels','#pixels'};%,'degree','%',
plotSaveOn=1;
dataSaveOn=1;

graphDir='/Users/ytun/Google Drive/IMPORTANT_VISION/Graphs';
subpathToSave=fullfile(graphDir,folderName);

if plotSaveOn
    mkdir(subpathToSave);
end

% if saveOn
% mkdir(subpathToSave);
% end

subModelNameList={''};%'dark20Test_';%darkTest_';%'rerun2_testind_';
linestylelist={'-','--','o-','*-'};

nUsers=length(subList);

%iGaze_err_deg=(91*5+10*8+11)/100; %page 9

% CHANGE HERE:
%modelName='ann'; % 'ANN' %'both'

%Compare line vs. ann (only line frames)
%or compare ann vs. cider (all frames)

subFolderName_list={'AllFramesCmpr'};
stage_list={'switching'};%,'IdealLine'};

modelvsStrList={'ANN'};

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
                    errData.(stage_list{mdInd}).(modelName).(objToPlotList{objInd}).indper=annIndPerAllSub;
                end
                
            end
            
           
            
            error2modelMat=[[annMeanErrorAllSub 0]'];% [lineMeanErrorAllSub 0]' ];
            std2modelMat=1.96*(1/sqrt(nUsers))*[[annStdErrorAllSub 0]'];% [lineStdErrorAllSub 0]' ];
            
              
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
            legendlist={'ANN'};
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
        
        
        
    end
    
    if dataSaveOn
        dir=fullfile('/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err',lighting);
        mkdir(dir);
        save(fullfile(dir,strcat(lighting,'_error_pplot.mat')),'errData');%,%'lineUsed_Lambdas_mean');
        
    end
    
%     if saveTableOn
%         writetable(tableSum_ANN,fullfile(dir,'finalTable',strcat(stage_list{i},lighting,'_tableSUM_ANN.csv')));
%         writetable(tableSum_Cider,fullfile(dir,'finalTable',strcat(stage_list{i},lighting,'_tableSUM_Cider.csv')));
%     end
    
end




%% ANN used


