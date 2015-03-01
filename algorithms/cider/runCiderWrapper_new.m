%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)

close all;
clear;

labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil_wrong';
dataRootDir='~/iShadow/algorithms/cider/data/new';

%cd ~
addpath ('~/iShadow/algorithms/cider/')

% % % Change here%

irbFolderCell={'awesomeness_parallel_test'};
%irbFolderCell={'awesomeness_irb_pupil_percent_0-2_98','awesomeness_irb_pupil_percent_0.05','awesomeness_parallel_test'};

modelCell={'cider','ann'};%,'ann'}; %'ann',
lightingList={'calib'};
%subLists={'addison','akshaya','shuo','duncan','mike','malai','kaituo'};
%uniList={'uniquefy_0','uniquefy_1'};
sub_ann='';

nDim=2;
scaleVect=[112 111];%[112 111 112];%

contrast_method='mean';
% % %
cd ~
cd (dataRootDir)


origin=pwd;

for n=1:length(irbFolderCell)
    
    irbFolderName=irbFolderCell{n};%strcat('awesomeness_irb_',object);
    cd(irbFolderName);
    
    origin_irb=pwd;
    
    subject_folders = cellstr(strsplit(ls));
    
    for i=2%1:length(subject_folders)-1
        
        subject_folder=subject_folders{i};
        
        originIRB=pwd;
        cd(subject_folder);
        
        uni_folders = cellstr(strsplit(ls));
        
        for k=1:length(uni_folders)-1
            do_uniquefy=k-1;
            cd(uni_folders{k});
            %cd(uni_folders{k});
            cd('results');
            
            %subFolderName=strcat(subName,'_',lighting,'.mat');
            input_dir=fullfile(labelPupilDir,strcat('addison_calib_pupil','.mat'));
            
            %result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'results');
            result_dir=pwd;%fullfile(dataRootDir,'data',irbFolderName,subFolderName,uni,'results');
            
            
            fprintf('Folder Name: %s\n',subject_folder);
            fprintf('Input dir: \n%s\n',input_dir);
            fprintf('ANN dir: \n%s\n',result_dir);
            
            %avgRadEllipse=out_data.avgRadEllipse(1:600,:);%(index_out,:);%(1:10,:);
            for m=1:length(modelCell)
                
                modelName=modelCell{m}; %'ann' or 'cider'
                
                fprintf('%s-%s-%s-%s',irbFolderName,subject_folder,modelName,uni_folders{k});
                
                if strcmp(modelName,'ann')
                    run_ann_sweep(result_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_ann,contrast_method);
                    
                elseif strcmp(modelName,'cider')
                    run_cider_sweep(result_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_ann,contrast_method);
                end
            end
            
            cd('../..')
        end
        cd('..')
    end
    cd('..');
end

fprintf('FINISHED\n');


