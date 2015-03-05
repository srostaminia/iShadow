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

subLists={'malai'}; %'akshaya','shuo','duncan','mike','shuo',

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
        
        subFolderName=strcat(subName,'_','outdoors','_pupil');
        out_data=load(fullfile(labelPupilDir,subFolderName));
        
        subFolderName=strcat(subName,'_','dark','_pupil');
        in_data=load(fullfile(labelPupilDir,subFolderName));
        
        subFolderName=strcat(subName,'_','outdoors','_pupil');
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
        figureOn=0;
       
    end
    
    fprintf('\n\nStarted %s\n\n',subName)
    
    %
    index_out=1087;
    
    for index_out=1:600
    
    X_out=out_data.X(index_out,:);%(1:10,:);%(1:600,:);%(1:100,:);%(1:100,:);
    
    
    X_out(:,111*[1:4]+48 )=repmat(mean(X_out,2),[1 4]);
    
    
    X_out(:,111*[0:111]+96 )=X_out(:,111*[0:111]+97 );
    
    filterOn=0;
    
    if filterOn
        hsize=3;
        imSample=mat2gray(reshape((X_out)',111, 112,[]));
        imSample=mat2gray((medfilt2(imSample,[hsize hsize])));
        X_out=(reshape(imSample,[], 111*112));
    end
    
    push=0;
    if push
        im_out=(reshape((X_out)',111, 112,[]));
        im_out=circshift(im_out,[]);
        X_out=(reshape(im_out,[], 111*112)); 
    end
    
    index_in=1087;
    X_in=in_data.X(index_in,:);%(1:10,:);%(1:600,:);%(1:100,:);%(1:100,:);

    X_in(X_in<200)=mean(X_in,2);
    
    
    X_out=mat2gray(X_out).*(max(X_in(:))-min(X_in(:))) +min(X_in(:));
      % X_out=mat2gray(X_out).*(max(X_in(:))-220) +220;
 
    addpath('~/iShadow/algorithms/ann/lib');

    
    if figureOn
    figure;
    subplot(2,3,1);
    histogram(X_out);
    title('raw: out hist');
    
    subplot(2,3,2);
    histogram(X_in);
    title('raw: in hist');
    
    end
    hsize=3;
    
    %imSample=mat2gray(medfilt2(imSample,[hsize hsize]));
    
    %imSample(1 )=0;
    imIn=mat2gray(reshape((X_in)',111, 112,[]));
    
    imSample=mat2gray(reshape((X_out)',111, 112,[]));
    imSample=histeq(imSample,imhist(imIn)); %%careful: histeq only allows imhist() which must be normalized
    X_out=(reshape(imSample,[], 111*112));
    X_out=mat2gray(X_out).*(max(X_in(:))-min(X_in(:))) +min(X_in(:));

    % % %
    X(index_out,:)=X_out;%(reshape(imSample,[], 111*112));
    gout(index_out,:)=out_data.gout(index_out,:);%(1:10,:);%(1:600,:);%(1:10,:);%(1:100,:);
    %avgRadEllipse=out_data.avgRadEllipse(index_out,:);%(1:10,:);
    % % %

    imSample=mat2gray(reshape(X_out',111, 112,[]));
    imIn=mat2gray(reshape(X_in',111, 112,[]));

    if figureOn
    subplot(2,3,3);
    %histogram(mean_contrast_adjust_nosave(X_out));
    histogram(X_in);
    title('in hist');
    
    subplot(2,3,4);
    %histogram(mean_contrast_adjust_nosave(X_out));
    histogram(X_out);
    title('out hist after normalizing');
    
    
    subplot(2,3,5);
    imshow(imSample);
    title('out after hist eq');
    
    
    subplot(2,3,6);
   % histogram(mean_contrast_adjust_nosave(X_in));
    %imSample=mat2gray(reshape(mean_contrast_adjust_nosave(X_in)',111, 112,[]));
     %imshow(imSample);
    % histogram(imSample);
    imshow(imIn);
    title('in image');
    
    suptitle('Feb 13- hist really eq');
    
    end
    fprintf('ss');
    end
   
   
    %%
    
%     im=mat2gray(reshape(X(10,:)',111, 112,[]));
%     row=im(45,:);
%     
%     figure;
%     subplot(4,1,1);
%     imshow(im);
%     hold on;
%     line([0 112],[45 45]);
%     hold off;
%     title('original outdoor image');
%     
%     
%     subplot(4,1,2);
%     imshow(row);
%     title('blue row');
%     
%     binr=im2bw(row);
%     
%     subplot(4,1,3);
%     imad=imadjust(row);
%     imshow(imad);
%     title('blue row after contrast adjustment');
%     
%     subplot(4,1,4);
%     imshow(im2bw(imad));
%     title('threshold');


%     hsize=[3 3];
%     sigma=0.5;
%     fspecial('gaussian', hsize, sigma) 

    %imshow(edge(binr,'canny'));
    
    %a=edge(im2bw(im(40:47,:)),'canny');
    %imshow(a)
    
%     figure;
%    histogram(X(1,:));
%    title('INDOOR: histogram of pixel values before mean_contrast_adjust_nosave');
%    a=mean_contrast_adjust_nosave(X(1,:));
%    figure;
%    histogram(a);
%       title('INDOOR: histogram of pixel values after mean_contrast_adjust_nosave');

    %X=X(121:130,:);
    %gout=gout(121:130,:);
   
    
  
      
   
    
 
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

