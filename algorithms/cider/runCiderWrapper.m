%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)

clear;

%  nDim=3;
%  scaleVect=[112 111 112];
nDim=2;
scaleVect=[112 111];
irbFolderName='awesomeness_irb_radius_target';

addpath('~/iShadow/algorithms/cider');

dataRootDir='~/iShadow/algorithms/cider';

cd ~
cd (dataRootDir)

subLists={'duncan','mike','shuo'}; %'akshaya','shuo'

for i=1:length(subLists)
      
    subName=subLists{i};
    
    
    %%
%     if strcmp(subName,'addison')
%         result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results');
%         data=load(fullfile('data','eye_data_addison_microbench1_pupil_auto.mat'));
%         
%     elseif strcmp(subName,'yamin')
%         result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results');
%         data=load(fullfile('data','eye_data_yamin_1sm4_pupil_auto.mat'));
%         
%     else
%         
%         subFolderName=strcat(subName,'_calib_pupil');
%         data=load(fullfile('data',strcat(subFolderName,'.mat')));
%         result_dir=fullfile(dataRootDir,'data/awesomeness_irb_pupil',subFolderName,'uniquefy_0','results');
%         
%         
%     end
    %%
    if strcmp(subName,'addison')
        %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results');
        subFolderName=strcat(subName,'_benchmark_pupil');
        data=load(fullfile('data','eye_data_addison_microbench1_pupil_auto.mat'));
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
    elseif strcmp(subName,'yamin')
        %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results');
        %data=load(fullfile('data','eye_data_yamin_1sm4_pupil_auto.mat'));
        %??
    else
        
        subFolderName=strcat(subName,'_calib_pupil');
        data=load(fullfile('data',strcat(subFolderName,'.mat')));
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
        
    end
    
    fprintf('\n\nStarted %s\n\n',subName)
    run_cider_sweep(result_dir, data.X, data.gout,nDim,scaleVect);

end





%%
% function runCiderWrapper(subName)
% 
% %ground truth file: akshaya_calib_pupil
% %result file: cider_rep1.mat(for each lambda and rep)
% 
% 
% rootDir='~/iShadow/algorithms/cider';
% 
% cd ~
% cd (rootDir)
% 
% subFolderName=strcat(subName,'_calib_pupil');
% data=load(fullfile('data',strcat(subFolderName,'.mat')))
% result_dir=fullfile(rootDir,'data/awesomeness_irb_pupil',subFolderName,'uniquefy_0','results')
% 
% run_cider_sweep(result_dir, data.X, data.gout);

