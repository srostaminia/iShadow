

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
modelName='ann';

labeldir='~/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';
addpath(labeldir);

workingdir='~/iShadow/algorithms/evaluation/MisMatch';
addpath(workingdir);

dataRootDir='~/iShadow/algorithms/cider';

object='radius_target';

cd ~
cd (labeldir)

subLists={'shuo'}; %'akshaya','shuo''addison','yamin',

for i=1:length(subLists)
    
    subName=subLists{i};
    
    %     if strcmp(subName,'addison')
    %         subFolderName=fullfile(labeldir,'eye_data_addison_microbench1_pupil_auto');
    %         result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results');
    %
    %
    %     elseif strcmp(subName,'yamin')
    %         %result_dir=fullfile(dataRootDir,'data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results');
    %         %%???data=load(fullfile('data','eye_data_yamin_1sm4_pupil_auto.mat'));
    %
    %     else
    %
    if strcmp(subName,'addison')
        lighting='benchmark';
    else
        lighting='calib';
    end
    
    result_dir=fullfile(dataRootDir,strcat('data/awesomeness_irb_',object),strcat(subName,'_',lighting,'_pupil'),'uniquefy_0','results');
    
    
    %  end
    if strcmp(subName,'addison')
       subFolderName='eye_data_addison_microbench1_pupil_auto'; 
    else
        subFolderName=strcat(subName,'_',lighting,'_raw');
    
    end
    
    labelData=load(strcat(subFolderName,'_ellipse.mat'));
    % ellipse(bestFitsMat(:,3),bestFitsMat(:,4),bestFitsMat(:,5)*pi/180,bestFitsMat(:,1),bestFitsMat(:,2),'y');
    %(ra,rb,ang,x0,y0,C,Nb)
    
    %     ellCentX=scale*labelData.bestFitsMat(:,1);
    %     ellCentY=scale*labelData.bestFitsMat(:,2);
    %     ellMajorlen=2*scale*labelData.bestFitsMat(:,3);
    %     ellMinorlen=2*scale*labelData.bestFitsMat(:,4);
    %     ellPhi=labelData.bestFitsMat(:,5);
    %
    trueArea=pi*labelData.bestFitsMat(:,3).*labelData.bestFitsMat(:,4);
    
    
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
            
            load(rep_files{m},'radii');
            
            
            %trueArea(1:3)
            
            
            
            predArea=pi*radii.^2;
            %predArea(1:3)
            %mismatchArea(n)
            areaDiff=abs(predArea-trueArea);
            %areaDiff(1:3)
            %mismatchArea(n)
            %overlapArea(n)=round(scaledOverlapArea/scale^2)
            %trueArea(n)
            areaDiffPercent=100*areaDiff./trueArea;
            %areaDiffPercent(1:3)
            %fprintf('Mismatch Area=%i, True Area=%i, PercentMisMatch=%.2f',mismatchArea(n), trueArea(n), percentAreaMismatch(n));
            
            
            
            save(sprintf('ann_rep%d.mat',m),'trueArea','areaDiffPercent','predArea','-append');
            %pred=predicted center, gout=ground truth center,
            %radii=estimated radii
            %err=ann error
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

