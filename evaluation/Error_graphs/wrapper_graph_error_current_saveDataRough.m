clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evalucation/Error_graphs')
subList={'addison','malai','mike','shuo','duncan','kaituo'};%'addison',
folderName='awesomeness_irb_radius_target_mean';%_outdoorFiltMedDia100Train';%'awesomeness_irb_radius_target_outdoorFiltMedBox100Train';%'awesomeness_irb_radius_target_dark80%train';%'awesomeness_irb_target_outdoor';%'awesomeness_irb_radius_target_mean';%%irb_pupil';
rootDir=fullfile('~/iShadow/algorithms/cider/data/new',folderName);
addpath('~/iShadow/algorithms/cider/');
lightingSubFolder='_calib_pupil';%_ohsocoldFiltMedDia_pupil';%'_calib_pupil';%'_ohsocoldFiltMedBox_pupil';%'_calib_pupil';%;;%
uni='uniquefy_0';
objToPlotList={'pupilLoc-pixel','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
titleObjToPlotList={'pupil center error (pixels)','pupil radius error (pixels)'}; %pupilLoc_deg,pupilLoc_pixel,area,radius

unitList={'#pixels','#pixels'};%,'degree','%',
saveOn=0;

graphDir='/Users/ytun/Google Drive/IMPORTANT_VISION/Graphs';
subpathToSave=fullfile(graphDir,folderName);

if saveOn
    mkdir(subpathToSave);
end

% if saveOn
% mkdir(subpathToSave);
% end

subModelNameList={'','','idealLINE100Test_'};%'dark20Test_';%darkTest_';%'rerun2_testind_';
linestylelist={'-','--','o-','*-'};

nUsers=length(subList);

iGaze_err_deg=(91*5+10*8+11)/100; %page 9

% CHANGE HERE:
%modelName='ann'; % 'ANN' %'both'

%Compare line vs. ann (only line frames)
%or compare ann vs. cider (all frames)
compareLineList={0 1 2};

subFolderName_list={'AllFramesCmpr','lineCmpr','IdealLine'};

modelvsStrList={'ANN vs. CIDER','ANN vs. Cross','Ideal Cross'};

nLambda=10;

checkpass=length(subModelNameList)==length(compareLineList) && length(compareLineList)==length(subFolderName_list) && length(subFolderName_list)==length(modelvsStrList);

if ~checkpass
    fprintf('ERROR: check subModelNameList,compareLineList,subFolderName_list,modelvsStrList in settings');
else
    for mdInd=1:length(modelvsStrList)
        
        subModelName=subModelNameList{mdInd};
        
        for objInd=1:length(objToPlotList)
            
            %
            %
            %         if compareLineList{mdInd}==1
            %             pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,'lineCmpr'));
            %             fileSub=strcat('lineCmpr_',subModelName);
            %
            %         elseif compareLineList{mdInd}==0
            %             pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,'AllFramesCmpr'));
            %             fileSub=strcat('AllFramesCmpr_',subModelName);
            %
            %         elseif compareLineList{mdInd}==2
            pathToSave=fullfile(subpathToSave,objToPlotList{objInd},strcat(subModelName,subFolderName_list{mdInd}));
            fileSub=strcat(subFolderName_list{mdInd},'_',subModelName);
            
            %        % else
            %             fprintf('ERROR: fix compareLineList items.');
            %             return;
            %         end
            
            if saveOn
                mkdir(pathToSave);
            end
            
            %Error graphs for all subjects: ANN and Cider OVERLAP
            for indFile=1 %always 1
                figure;
                
                linestyle=linestylelist{indFile};
                %nLambda=10;
                
                if strcmp(modelvsStrList{mdInd},'Ideal Cross')
                    subModelName_ann='';
                else
                    subModelName_ann=subModelName;
                end
                
                modelName='ann'; % 'ANN' %'both'
                graph_error_model_current_meanOfmean_saveAllData(subFolderName_list{mdInd},indFile,linestylelist{1},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName_ann,compareLineList{mdInd});
                %nLambda=9;
                hold on;
                
                % CHANGE HERE:
                modelName='cider'; % 'ANN' %'both'
                graph_error_model_current_meanOfmean_saveAllData(subFolderName_list{mdInd},indFile,linestylelist{2},modelName,subList,rootDir,lightingSubFolder,objToPlotList{objInd},subModelName,compareLineList{mdInd});
                
                
            end
            
        end
    end
end

%% ANN used


