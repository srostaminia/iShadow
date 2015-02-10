%runciderPupilWrapper
%Yamin
%Jan 23, 2015

%ground truth file: akshaya_calib_pupil
%result file: cider_rep1.mat(for each lambda and rep)


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
        
        data=load(fullfile(labelPupilDir,strcat(subFolderName,'.mat')));
        result_dir=fullfile(dataRootDir,'data',irbFolderName,subFolderName,'uniquefy_0','results');
        
    end
    
    fprintf('\n\nStarted %s\n\n',subName)
    
    
    
    %
    X=data.X;
    gout=data.gout;
     
    X=X(1:10,:);
    gout=gout(1:10,:);
    
%     if strcmp(lighting,'outdoors')
%         
%         addpath('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data');
%         
%         dataCalib=load(fullfile(labelPupilDir,strcat(subName,'_calib_pupil.mat')));
%         
%         b=dataCalib.X(1,:);
%         imSample=mat2gray(reshape(b',111, 112,[]));
%         %imhist(imSample)
%         
%         %imread('eye_center.png');
%         %imSample=mat2gray(rgb2gray(imread(strcat(subName,'_','calib','.png'))));
%         newhis=(imhist(imSample));
%         
%         %     figure;
%         %     imshow(imSample);
%         %     title('Sample Image');
% 
%         X(:,[111*1+48 111*2+48 111*3+48 111*4+48] )=repmat(mean(X,2),[1 4]);
%         
%         
%       %  im=(reshape(X(1,:)',111, 112,[]));
%         
% %         figure;
% %         imshow(mat2gray(im));
%          
%         
%         %remove blinks
%         %if strcmp(subName,'duncan')
%         X=X(1:10,:);
%         gout=gout(1:10,:);
%         
%         for p=1:10
%             a=mat2gray(X(p,:));
%             X(p,:)=histeq(a,newhis);
%         end
%         
%         
%         %im=(reshape(X',111, 112,[]));
%         
% %         for z=1:1
% %             figure;
% %             imshow(im(:,:,z));
% %             title(sprintf('%i, after hist eq',z));
% %             
% %         end
%         
%         fprintf('data cropped');
%         
%         %              X=X([1:20],:);
%         %              gout=gout([1:20],:);
%         %             X([21:23 54:55 64 85 204 345 493 552],:)=[];
%         %             gout([21:23 54:55 64 85 204 345 493 552],:)=[];
%         
%         %end
%         
% %         imArr=reshape(X',111, 112,[]);
% %         
% %         
% %         figure;
% %         imshow(imSample);
% %         title('calib')
% %         %colormap gray;
% %         fprintf('yes');
% %         
% %         figure;
% %         imshow(imArr(:,:,1));
% %         title('adjusted')
% %         %colormap gray;
% %         fprintf('yes');
%     end
%  
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

