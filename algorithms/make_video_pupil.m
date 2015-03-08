close all;
clear;

%% Edit here:
lighting='ohsocoldFiltMedDia';

modelName='ann';
object='target_outdoor';


indexEnd=10;%nImages;

subList={'addison'}; %'addison',,'duncan'
shuttleVideo = VideoReader('shuttle.avi');
    
    
for s=1:length(subList)
    
    subName=subList{s};
    datastr=strcat(subName,'_',lighting,'_pupil');%'eye_data_addison_microbench1_pupil_auto';%strcat(subName,'_',lighting,'_raw');
    ellipseFileName = strcat(datastr,'_ellipse.mat');
    dataFileName=strcat(datastr,'.mat');
    
    fileName=strcat(subName,'_',lighting,'_pupil');
    %%
    labelPupildir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';
    addpath(labelPupildir);
    
    datadir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/';
    % addpath(datadir);
    
    addpath('/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/');
    
    videoDir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelvideo/';
    addpath(videoDir);
    
    resultDir=fullfile('~/iShadow/algorithms/cider/data/new',strcat('awesomeness_irb_',object),fileName,'results/subset_l1_init_strips_k7_lambda0.010000');
    
    
    
    %%
    
   
%     modelName='cider';
%     data=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')));
%     
%     filter=data.radii.true>0 & logical(sum(data.center.pred,2)) & data.radii.pred>0 & ~data.ann_used;
%     data.center.true=data.center.true(filter,:);
%     data.center.pred=data.center.pred(filter,:);
%     data.radii.pred=data.radii.pred(filter,:);
%     data.radii.true=data.radii.true(filter,:);
%     
%     labelPupil_data=load(fullfile(labelPupildir,strcat(subName,'_',lighting,'_pupil.mat')));   
%     nImages=sum(filter);
%     image=labelPupil_data.X(filter,:);
%     imArr=reshape(image',111, 112,[]);
% 
%     bestFitsMat=labelPupil_data.bestFitsMat(filter,:);
%     
    modelName='ann';
    data=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')));
    
    filter=logical(sum(data.center.pred,2)) ;
    gout=data.center.true(filter,:);
    pred=data.center.pred(filter,:);
    
    labelPupil_data=load(fullfile(labelPupildir,strcat(subName,'_',lighting,'_pupil.mat')));   
    nImages=sum(filter);
    image=labelPupil_data.X(filter,:);
    imArr=reshape(image',111, 112,[]);

    bestFitsMat=labelPupil_data.bestFitsMat(filter,:);
    
    
    %%
    
    outputVideo = VideoWriter(fullfile(videoDir,strcat(subName,'_',lighting,'_','0.01results','.avi')));
    outputVideo.FrameRate = shuttleVideo.FrameRate;
    open(outputVideo)
    
    for i=1:5:1000%1:indexEnd%[1:100 200:300 400:500 1000:1200 1300:1400 1500:1600 ]
        %for i=[1:100 300:400 800:900 2200:2400 5000:5100 6000:6100 ]
        
        %Detect pupil
        im=imArr(:,:,i);
        imageName=int2str(i);
        
        bestFits=bestFitsMat(i,:);
        
        h = figure('visible', 'off');
        a = axes('parent', h);
        
        for j=1:3
            subplot(1,3,j);
            imagesc(im);
            axis square
            axis off
            colormap gray;
            
            hold on;
            
            if j==1 || j==3
                ellipse(bestFits(:,3),bestFits(:,4),bestFits(:,5)*pi/180,bestFits(:,1),bestFits(:,2),'y');
                plot(gout(i,1),gout(i,2),'oy','markersize',5);
                %plot(bestFits(:,1),bestFits(:,2),'oy','markersize',5);
            end
            
            if j==2 || j==3
                plot(pred(i,1),pred(i,2),'xb','markersize',5);
%            viscircles(data.center.pred(i,:),data.radii.pred(i,:),'EdgeColor','b','LineWidth',0.3);
%                 plot(data.center.pred(i,1),data.center.pred(i,2),'xb','markersize',5);
            end
            
             hold off;
            
            title({sprintf('%i',i),'blue- line','yellow- ground truth'});
            
        end
        img = frame2im(getframe(h));
        
        writeVideo(outputVideo,img);
        
        fprintf('%i\n',i);
        
    end
    
    
    
    close(outputVideo)
    
end