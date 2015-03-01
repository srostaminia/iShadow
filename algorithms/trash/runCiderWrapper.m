%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)

close all;
clear;
modelName='ann';
%nDim=3;
%scaleVect=[112 111 112];
nDim=2;
scaleVect=[112 111];
object='pupil';
irbFolderName=strcat('awesomeness_irb_',object,'_percent');

labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';
addpath('~/iShadow/algorithms/cider');

dataRootDir='~/iShadow/algorithms/cider';
lighting='benchmark_pupil';

cd ~
cd (dataRootDir)

subLists={'addison'}; %'akshaya','shuo','duncan','mike','shuo',

sub_ann='_percent';

contrast_method='percentile';

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
%     if strcmp(subName,'addison')
%         %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results');
%         subFolderName=strcat(subName,'_benchmark_',object);
%         data=load(fullfile('data','eye_data_addison_microbench1_pupil_auto.mat'));
%         result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
%         
%     elseif strcmp(subName,'yamin')
%         %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results');
%         %data=load(fullfile('data','eye_data_yamin_1sm4_pupil_auto.mat'));
%         %??
%     else
        
        %subFolderName=strcat(subName,'_',lighting);
        %out_data=load(fullfile(labelPupilDir,'addison_outdoors_test_eye.mat'));
        
        subFolderName=strcat(subName,'_',lighting,'.mat');
        fullfile(labelPupilDir,subFolderName)
        out_data=load(fullfile(labelPupilDir,subFolderName));
        
        subFolderName=strcat(subName,'_',lighting,'');
       
        %result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'results');
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        result_dir
        
   % end
   
    %index_out=;
    XX=out_data.X;
    %XX([17 22:26 64 203:204 239 287 487 552 561],:)=[];
    
%      %correct outdoor pixels
%      brokenPixels=(XX(:,111*[1:4]+48 ));
%     XX(:,111*[1:4]+48 )=repmat(mean(XX,2),[1 4]);
%     XX(:,111*[0:111]+96 )=XX(:,111*[0:111]+97 );
%     
%     figure;
%     imagesc(reshape((XX(1,:))',111,112,[]));
%     colormap gray
    
     ggout=out_data.gout;
    %ggout([17 22:26 64 203:204 239 287 487 552 561],:)=[];
    
    
    X=XX;%XX(1:600,:);%(index_out,:);%(reshape(imSample,[], 111*112));
    gout=ggout;%(index_out,:);%(1:10,:);%(1:600,:);%(1:10,:);%(1:100,:);
    %avgRadEllipse=out_data.avgRadEllipse(1:600,:);%(index_out,:);%(1:10,:);
   
    if strcmp(modelName,'ann')
      run_ann_sweep(result_dir, X, gout,nDim,scaleVect,sub_ann,contrast_method);
        
    elseif strcmp(modelName,'cider')
      run_cider_sweep(result_dir, X, gout,nDim,scaleVect,sub_ann,contrast_method);
    end
    

end




