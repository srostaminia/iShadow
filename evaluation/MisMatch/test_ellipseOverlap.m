

%%

%Yamin
%Jan 28, 2015

%zero radius->mismatch->
%over 100% mismatch->


clear;

%scale it to avoid inaccuracy due to pixelation
scale=1;
w=scale*112;
h=scale*111;

plotMode='none';
modelName='cider';

labeldir='~/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';
addpath(labeldir);

workingdir='~/iShadow/algorithms/evaluation/MisMatch';
addpath(workingdir);

dataRootDir='~/iShadow/algorithms/cider';


cd ~
cd (labeldir)

subLists={'duncan','mike','shuo','addison'}; %'akshaya','shuo''addison','yamin',

for i=1:length(subLists)
      
    subName=subLists{i};
    
    if strcmp(subName,'addison')
        subFolderName=fullfile(labeldir,'eye_data_addison_microbench1_radius_target_auto');
        result_dir=fullfile(dataRootDir,'data/awesomeness_radius_target_microbench1_uniquefy/addison_microbench1_radius_target/full/results');
        
        
    elseif strcmp(subName,'yamin')
        result_dir=fullfile(dataRootDir,'data/awesomeness_radius_target_1sm4_uniquefy/yamin_1sm4_radius_target/full/results');
        %%???data=load(fullfile('data','eye_data_yamin_1sm4_radius_target_auto.mat'));
        
    else
        
        subFolderName=strcat(subName,'_calib_raw');
        result_dir=fullfile(dataRootDir,'data/awesomeness_irb_radius_target',strcat(subName,'_calib_radius_target'),'uniquefy_0','results');
        
        
    end
    
    labelData=load(strcat(subFolderName,'_ellipse.mat'));
    % ellipse(bestFitsMat(:,3),bestFitsMat(:,4),bestFitsMat(:,5)*pi/180,bestFitsMat(:,1),bestFitsMat(:,2),'y');
    %(ra,rb,ang,x0,y0,C,Nb)
    
    ellCentX=scale*labelData.bestFitsMat(:,1);
    ellCentY=scale*labelData.bestFitsMat(:,2);
    ellMajorlen=2*scale*labelData.bestFitsMat(:,3);
    ellMinorlen=2*scale*labelData.bestFitsMat(:,4);
    ellPhi=labelData.bestFitsMat(:,5);
    
%     pi*ellMajorlen(1)*ellMinorlen(1)/scale^2
%     pi*ellMajorlen(2)*ellMinorlen(2)/scale^2
     trueArea=pi*labelData.bestFitsMat(:,3).*labelData.bestFitsMat(:,4);
%     trueArea(1:2)
        

    %[x0 y0 a b alpha score] being the center of the ellipse, its major and minor axis, its angle in degrees and score.
    
    fprintf('\n\nStarted %s\n\n',subName)
    
    origin = pwd;
    
    cd(result_dir);
    lambda_folders = cellstr(strsplit(ls));
    
    for k=1:length(lambda_folders)-1
        cd(lambda_folders{k});
        
        rep_files = cellstr(strsplit(ls(strcat(modelName,'*'))));
        
        for m=1:length(rep_files)-1
            fprintf('%s - %d\n',lambda_folders{k},m);
            
            load(rep_files{m},'radii','pred');
            
             radii=scale*radii;
             pred=scale*pred;           
            
             nFrame=size(radii,1);
             
             percentAreaMismatch=zeros(nFrame,1);
             mismatchArea=zeros(nFrame,1);
             %trueArea=zeros(nFrame,1);
             
               %zero radius->mismatch->
               %over 100% mismatch->
             for n=1:nFrame
                 circleSt.radius=radii(n);%1.5*w/10;
                 
                 %n
                 %fprintf('n=%i\n',n);
                 
                 if circleSt.radius>0
                     circleSt.centX=pred(n,1);
                     circleSt.centY=pred(n,2);
                     
                     %trueArea(n)=pi*ellipseSt.majorlen*ellipseSt.minorlen/scale^2;
                     
                     
                     ellipseSt.centX=ellCentX(n);
                     ellipseSt.centY=ellCentY(n);
                     ellipseSt.majorlen=ellMajorlen(n);
                     ellipseSt.minorlen=ellMinorlen(n);
                     ellipseSt.phi=ellPhi(n);
                     %circleSt
                     %ellipseSt
                     
                     [scaledMismatchArea,~,elArea]=getMismatchAreaEC(w,h, circleSt, ellipseSt,plotMode);
                     %size(scaledMismatchArea)
                     %n
                     %mismatchArea(n)
                     mismatchArea(n)=round(scaledMismatchArea/scale^2);
                     %mismatchArea(n)
                     %overlapArea(n)=round(scaledOverlapArea/scale^2)
                     %trueArea(n)
                     percentAreaMismatch(n)=100*mismatchArea(n)/trueArea(n);
                     
                     %fprintf('Mismatch Area=%i, True Area=%i, PercentMisMatch=%.2f',mismatchArea(n), trueArea(n), percentAreaMismatch(n));
                 end
                 
                 clearvars circleSt ellipseSt
             end
             
             

            save(sprintf('cider_rep%d.mat',m),'percentAreaMismatch','trueArea','mismatchArea','-append');
            %pred=predicted center, gout=ground truth center,
            %radii=estimated radii
            %err=cider error
            %err_Line=line model error
        end
        
        cd('..');
    end
    
    cd(origin);
    
    fprintf('FINISHED');


end





%% Test script
% clear;
% close all;
% 
% plotMode='all';
% 
% dir='/Users/ytun/Documents/Dropbox/Vision/MobiSys2015_labeling/label'
% filename='malai_dark_raw_ellipse.mat'
% 
% data=fullfile(dir,filename);
% 

% %%
% 
% %Test script
% addpath('/Users/ytun/iShadow/evaluation/MisMatch');
% 
% %scale it to avoid inaccuracy due to pixelation
% scale=20;
% 
% w=scale*112;
% h=scale*111;
% 
% circleSt.radius=1.5*w/10;
% circleSt.centX=w/2;
% circleSt.centY=h/2;
% 
% ellipseSt.centX=100;%w/10;%w/2;%10+w/2;
% ellipseSt.centY=50;%w/10;%h/2;10+w/2;
% ellipseSt.majorlen=2*w/10;
% ellipseSt.minorlen=w/10;
% ellipseSt.phi=45;
% 
% [mismatchArea]=getOverlapAreaEC(w,h, circleSt, ellipseSt,plotMode);
% trueArea=pi*ellipseSt.majorlen*ellipseSt.minorlen/scale^2;
% 
% mismatchArea=round(mismatchArea/scale^2);
% 
% percentAreaMismatch=100*mismatchArea/trueArea;
% 
% %if no overlap
% %percentAreaMismatch(percentAreaMismatch>100)=-100;
% 
% % trueArea
% % mismatchArea
% % percentAreaMismatch
% 

%%

