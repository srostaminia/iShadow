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
irbFolderName=strcat('awesomeness_irb_',object);

labelPupilDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';
addpath('~/iShadow/algorithms/cider');

dataRootDir='~/iShadow/algorithms/cider';
lighting='outdoors';

cd ~
cd (dataRootDir)

subLists={'duncan','malai'}; %'akshaya','shuo','duncan','mike','shuo',

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
        subFolderName=strcat(subName,'_benchmark_',object);
        data=load(fullfile('data','eye_data_addison_microbench1_pupil_auto.mat'));
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
    elseif strcmp(subName,'yamin')
        %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results');
        %data=load(fullfile('data','eye_data_yamin_1sm4_pupil_auto.mat'));
        %??
    else
        
        subFolderName=strcat(subName,'_',lighting,'_pupil');
        
        data=load(fullfile(labelPupilDir,subFolderName));
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
    end
    
    fprintf('\n\nStarted %s\n\n',subName)
    
    
    
    %
    X=data.X(1:10,:);%(1:100,:);
    gout=data.gout(1:10,:);%(1:100,:);
     
    %X=X(121:130,:);
    %gout=gout(121:130,:);
    
    if strcmp(lighting,'outdoors')
        
        addpath('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data');
        
        dataCalib=load(fullfile(labelPupilDir,strcat(subName,'_calib_pupil.mat')));
        
        b=dataCalib.X(10,:);
        maxx=max(b);

        imSample=mat2gray(reshape(b',111, 112,[]));
        %imSample=mat2gray(rgb2gray(imread(strcat(subName,'_','calib','.png'))));
        newhis=(imhist(imSample));
        
      %  figure;
       % imhist(imSample);
        %imshow(imSample);
        %title('Sample Image');

        X(:,[111*1+48 111*2+48 111*3+48 111*4+48] )=repmat(mean(X,2),[1 4]);
         addpath('~/iShadow/algorithms/ann/lib');

         hsize=3;
        
        for p=1:size(X,1)
%             figure;
%             imhist(X(p,:))
%             before=imhist(X(p,:));
             
            a=mat2gray(X(p,:));
            X(p,:)=histeq(a,newhis);
            
%             figure;
%             imhist(X(p,:))
%             after=imhist(X(p,:));
%             
            X(p,:)=maxx*X(p,:);
            
            im=(reshape(X(p,:)',111, 112,[]));
            imadjusted=medfilt2(im,[hsize hsize]);
             figure;
%             imhist(X(p,:));
            subplot(1,2,1);
            imagesc(im);
            colormap gray
            title('before med filter');
            subplot(1,2,2);
            imagesc(imadjusted);
            colormap gray
            title('after med filter');

            %im=(reshape(X(p,:)',111, 112,[]));
        end
        
        
        %im=(reshape(X',111, 112,[]));
        

        
        fprintf('data cropped');
      
%         
%         
%         figure;
%         imshow(imSample);
%         title('calib')
%         %colormap gray;
%         fprintf('yes');
%         
%         figure;
%         imshow(imArr(:,:,1));
%         title('adjusted')
%         %colormap gray;
%         fprintf('yes');
    end
 
    if strcmp(modelName,'ann')
        run_ann_sweep(result_dir, X, gout,nDim,scaleVect);
        
    elseif strcmp(modelName,'cider')
        run_cider_sweep(result_dir, X, gout,nDim,scaleVect);
    end
    
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

