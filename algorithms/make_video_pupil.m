close all;
clear;

%% Edit here:
namesub='addison';
lighting='benchmark';
result='predAndTrue';

modelName='cider';
object='radius_target';
datastr='eye_data_addison_microbench1_pupil_auto';%strcat(namesub,'_',lighting,'_raw');
ellipseFileName = strcat(datastr,'_ellipse.mat');

dataFileName=strcat(datastr,'.mat');

indexEnd=800;%nImages;

fileName='addison_benchmark_pupil';
%%
labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';
addpath(labeldir);

datadir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data/';
% addpath(datadir);

videoDir='~/iShadow/algorithms/video';
addpath(videoDir);

resultDir=fullfile('~/iShadow/algorithms/cider/data',strcat('awesomeness_irb_',object),fileName,'uniquefy_0/results/subset_l1_init_strips_k7_lambda0.000100');



%%

if strcmp(object,'radius_target')
    modelName='cider';
    cider=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')),'pred','gout','radii');   
    modelName='ann';
    ann=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')),'pred','gout','radii');
    
elseif strcmp(object,'pupil')
    modelName='cider';
    cider=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')));
    modelName='ann';
    ann=load(fullfile(resultDir,strcat(modelName,'_rep1.mat')));

end


data=load(fullfile(datadir,dataFileName));
image=data.X;
imArr=reshape(image',111, 112,[]);
nImages=size(imArr,3);

labelellipse=load (fullfile(labeldir,ellipseFileName));

bestFitsMat=labelellipse.bestFitsMat;

%%
shuttleVideo = VideoReader('shuttle.avi');

outputVideo = VideoWriter(fullfile(videoDir,strcat(namesub,'_',lighting,'_',result,'.avi')));
outputVideo.FrameRate = shuttleVideo.FrameRate;
open(outputVideo)




for i=1:indexEnd%[1:100 200:300 400:500 1000:1200 1300:1400 1500:1600 ]
    %for i=[1:100 300:400 800:900 2200:2400 5000:5100 6000:6100 ]

    %Detect pupil
    im=imArr(:,:,i);
    imageName=int2str(i);


    bestFits=bestFitsMat(i,:);


    h = figure('visible', 'off');
    a = axes('parent', h);
    imagesc(im);
    axis square
    axis off
    colormap gray;
    
    hold on;
    ellipse(bestFits(:,3),bestFits(:,4),bestFits(:,5)*pi/180,bestFits(:,1),bestFits(:,2),'y');
    scatter(bestFits(:,1),bestFits(:,2),'xy');
    
%     if strcmp(object,'radius_target')
%        radius=cider.radii(i,:);
%     else
%         radius=5;
%     end
    
    viscircles(cider.pred(i,:), cider.radii(i,:),'EdgeColor','r','LineWidth',0.3);
    scatter(cider.pred(i,1),cider.pred(i,2),'xr');
   
    viscircles(ann.pred(i,:), ann.radii(i,:),'EdgeColor','b','LineWidth',0.3);
    scatter(ann.pred(i,1),ann.pred(i,2),'xb');
    hold off;
    
    
    
    title({sprintf('%i',i),'blue- ann','red- cider','yellow- ground truth'});
    
    img = frame2im(getframe(h));

    writeVideo(outputVideo,img);

    fprintf('%i\n',i);

end



close(outputVideo)