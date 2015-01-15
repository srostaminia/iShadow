function runCiderWrapper(subName)



rootDir='~/iShadow/algorithms/cider';

subFolderName=strcat(subName,'_calib_pupil');
data=load(fullfile('data',strcat(subFolderName,'.mat')))

result_dir=fullfile(rootDir,'data/awesomeness_irb_pupil',subFolderName,'uniquefy_0','results')


run_cider_sweep(result_dir, data.X, data.gout)