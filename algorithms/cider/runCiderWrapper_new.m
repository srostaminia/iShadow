%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)

close all;
clear;

labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';
dataRootDir='~/Documents/ISHADOW_DATA/new';
%cd ~
addpath ('~/iShadow/algorithms/cider/')

% % % Change here%
irbFolderCell={'awesomeness_irb_radius_target_CALIBrun1'};%,'awesomeness_irb_radius_target_CALIBcrossval'};%dark80%train'};%{'awesomeness_irb_radius_target_outdoorFiltMedDia100Train'};%'awesomeness_irb_radius_target_dark80%train'};%'awesomeness_irb_radius_target_outdoorFiltMedBox100Train','awesomeness_irb_radius_target_outdoorRaw100Train'};
%irbFolderCell={'awesomeness_irb_pupil_percent_0-2_98','awesomeness_irb_pupil_percent_0.05','awesomeness_parallel_test'};

modelCell={'ann','cider'};%,'cider'};%,'idealLine'};%,'ann'};%,'cider'};%,'ann'}; %'ann',
lighting='calib';%{'dark','dark','dark'};%'ohsocoldFiltMedDia'};%{'dark','calib'};%{'ohsocoldFiltMedBox','ohsocold'};
subLists={'pan','pengyu'};%,'shuo','mike','malai','duncan','kaituo'};%,'duncan','kaituo''akshaya','shuo','duncan','mike','malai','kaituo'};%'addison',
uni_folders={'uniquefy_0'};
sub_modelList={'',''};%,'','idealLINE_'};%,'idealLINE_'};%'dark20Test_','calib100Test_'}%{''};%{'dark20Test_','calib100Test_'};%darkTest_';

diffStr='sameANN_submodel';%'sameANN_submodel'; %diffANN_irb, sameANN_submodel
%testIndOnList={'randomCross','randomCross','randomCross'};%{'8020','100'};%{'100'};%{'8020','100'}; %randomCross

nDim=3; %3
scaleVect=[112 111 112];% 112];%[112 111 112];%

percentile=0;

idealOn=0; %gout as previous prediction

thirdVarName='radius';

contrast_method='mean';
% % %
cd ~
cd (dataRootDir)

origin=pwd;

if strcmp(diffStr,'sameANN_submodel')
    checkpass=(length(modelCell))==length(sub_modelList);
elseif strcmp(diffStr,'diffANN_irb')
    fprintf('ERROR: You need to modify code');
    return;
   % checkpass=(length(irbFolderCell)==length(lightingList));
else
    fprintf('ERROR: fix power graph.');
end

if checkpass
    % for m=1:length(testIndOnList)
    for n=1:length(irbFolderCell)
        
        irbFolderName=irbFolderCell{n};%strcat('awesomeness_irb_',object);
        cd(irbFolderName);
        
        origin_irb=pwd;
        
        subject_folders = cellstr(strsplit(ls));
        
        for i=1:length(subject_folders)-1
            
            subject_folder=subject_folders{i};
            
            strList=strsplit(subject_folder,'_');
            subName=strList{1};
            
            originIRB=pwd;
            cd(subject_folder);
            
            %uni_folders = cellstr(strsplit(ls));
            
            %for k=1:length(uni_folders)-1
            k=1;
            do_uniquefy=k-1;
            %cd(uni_folders{k});
            %cd(uni_folders{k});
            cd('results');
            
            if strcmp(diffStr,'sameANN_submodel')
                %subFolderName=strcat(subName,'_',lighting,'.mat');
                input_dir=fullfile(labelPupilDir,strcat(subName,'_',lighting,'_pupil.mat'));%strcat('addison_calib_pupil','.mat'));
            elseif strcmp(diffStr,'diffANN_irb')
                input_dir=fullfile(labelPupilDir,strcat(subName,'_',lighting,'_pupil.mat'));%strcat('addison_calib_pupil','.mat'));
            end
            
            %result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'results');
            result_dir=pwd;%fullfile(dataRootDir,'data',irbFolderName,subFolderName,uni,'results');
            
            fprintf('Folder Name: %s\n',subject_folder);
            fprintf('Input dir: \n%s\n',input_dir);
            fprintf('ANN dir: \n%s\n',result_dir);
            
            %avgRadEllipse=out_data.avgRadEllipse(1:600,:);%(index_out,:);%(1:10,:);
            for p=1:length(modelCell)
                
                modelName=modelCell{p}; %'ann' or 'cider'
                
                fprintf('%s-%s-%s-%s-%s',sub_modelList{p},irbFolderName,subject_folder,modelName,uni_folders{k});
                
                %                 if strcmp(modelName,'ann')
                %                     run_ann_sweep(result_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_ann,contrast_method);
                %
                %                 elseif strcmp(modelName,'cider')
                %                     run_cider_sweep(result_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_ann,contrast_method);
                %                 end
                %if strcmp(modelCell{m},'ann') && sub_modelList
                run_annOrCider_sweep(modelName,result_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_modelList{p},contrast_method,percentile,'randomCross',thirdVarName);
                
                %end
            end
            cd('../..')
            %end
            %cd('..')
        end
        cd('..');
    end
    cd (origin);
    % end
else
    fprintf('ERROR: check testIndOnList, lightingList and sub_modelList');
end

fprintf('\nFINISHED\n');


