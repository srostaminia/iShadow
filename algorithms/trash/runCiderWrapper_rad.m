%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)

clear;

addpath('~/iShadow/algorithms/cider');

dataRootDir='~/iShadow/algorithms/cider';

cd ~
cd (dataRootDir)

subLists={'addison','shuo','mike','duncan'}; %'akshaya','shuo'
obj='radius';

for i=1:length(subLists)
    
    subName=subLists{i};
    
    if strcmp(subName,'addison')
        subFolderName=strcat(subName,'_benchmark_pupil');
        data=load(fullfile('data','eye_data_addison_microbench1_pupil_auto.mat'));
    else
        subFolderName=strcat(subName,'_calib_pupil');
        data=load(fullfile('data',strcat(subFolderName,'.mat')));
    end
    
    
    subFolderName=strcat(subFolderName,'_v7');
    result_dir=fullfile(dataRootDir,'data',strcat('awesomeness_irb_',obj),subFolderName,'uniquefy_0','results');
    
    
    
    fprintf('\n\nStarted %s\n\n',subName)
    run_cider_sweep(result_dir, data.X, data.gout);

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

