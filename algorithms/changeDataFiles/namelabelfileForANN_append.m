
%% change version to v7 to save space and add radius of ellipse fit- average of major minor axis/2
clear;
labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';
labelPupildir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/gazeLabel';
datadir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/gazeData';

% load(fullfile(datadir,'file.mat'));
% save(fullfile(datadir,'file_v7.mat'),'gout','X','-v7');

names={'addison'};%,'kaituo','malai','sean','akshaya','mike','shuo','duncan'};
% lightings={'calib','dark'};
%names={'akshaya','mike','shuo','duncan'};%{'kaituo','malai','sean'};
lightings={'calib'};%,'dark'};

%filter_Line = logical(sum(pred,2)) & ~ann_used;
%dist_Line = sqrt(sum((gout(filter_Line) - pred(filter_Line)).^2,2));
%err_Line = mean(dist_Line);

object='gaze'; %'pupil'

varName1='centerMat';
varName2='radiiMat';


for z=1:length(names)
    
    name=names{z};
    
    for y=1:length(lightings)
        
        lighting=lightings{y};
        
        if strcmp(name,'addison')
%             if y==1
%                 filename=strcat('addison_benchmark_',object,'.mat');%'outdoors_final_eye_ellipse_wed-3-38.mat';
%             elseif y==2
%                 filename=strcat('addison_outdoor_',object,'.mat');%'outdoors_final_eye_ellipse_wed-3-38.mat';
% 
%             end
filenameLabel='addison_microbench1_raw_gazeLabel.mat';
        else
            filename=strcat(name,'_',lighting,'_',object,'.mat');
        end
        
        filenameData='addison_microbench1_raw.mat';
        %a='eye_data_addison_microbench1_pupil_auto_ellipse.mat';%'outdoors_final_eye_ellipse_wed-3-38.mat';
        load(fullfile(labelPupildir,filenameLabel),varName1);
        
        %load(fullfile(datadir,filenameData),'X');

                labelEll=load(fullfile(labeldir,a),'avgRadEllipse');
        %         bestFitsMat=labelEll.bestFitsMat;
        %         %1st var
        %        avgRadEllipse=(bestFitsMat(:,3)+bestFitsMat(:,4))/4; % /2 for radius, another /2 for averaging
        %
        %         %b='addison_benchmark_pupil.mat';%'addison_outdoor_pupil.mat';
        %         b=strcat(name,'_',lighting,'_pupil.mat');
        %         %2nd and 3rd vars
        %         label=load(fullfile(labelPupildir,b));
        %         gout=label.gout;
        %         X=label.X;
        
        %filename='addison_benchmark_pupil.mat';
        %avgRadEllipse=avgRadEllipse*2;
        %gout=centerMat;
        
        %save(fullfile(labelPupildir,'addison_benchmark_gaze.mat'),'gout','X');
        save(fullfile(labelPupildir,filename),'avgRadEllipse','-append');
        
        
    end
end

fprintf('succeed')

% %% Convert to V7- default
% % no need to convert
% 
% cd ~
% cd '/Users/ytun/Google Drive/Vision/MobiSys2015_labeling/labelPupil/'
% 
% %files={'kaituo','malai','sean'};
% clear;
% 
% load('addison_benchmark_pupil_v7.mat');
% save('addison_benchmark_pupil.mat','-v7');
