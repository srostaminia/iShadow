
%% change version to v7 to save space and add radius of ellipse fit- average of major minor axis/2
clear;
labeldir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/label';
labelPupildir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil';

datadir='/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/data';

% load(fullfile(datadir,'file.mat'));
% save(fullfile(datadir,'file_v7.mat'),'gout','X','-v7');

% names={'kaituo','malai','sean'};%akshaya,'mike','shuo','duncan'};
% lightings={'calib','dark'};
names={'malai'};%{'akshaya','mike','shuo','duncan'};%{'kaituo','malai','sean'};
lightings={'dark'};

% filter_Line = logical(sum(pred,2)) & ~ann_used;
% dist_Line = sqrt(sum((gout(filter_Line) - pred(filter_Line)).^2,2));
% err_Line = mean(dist_Line);

for z=1:length(names)
    
    name=names{z};
    
    for y=1:length(lightings)
        
        lighting=lightings{y};
        
        
        %a='eye_data_addison_microbench1_pupil_auto_ellipse.mat';%'outdoors_final_eye_ellipse_wed-3-38.mat';
        a=strcat(name,'_',lighting,'_raw_ellipse.mat');
        labelEll=load(fullfile(labeldir,a));
        bestFitsMat=labelEll.bestFitsMat;
        %1st var
        avgRadEllipse=(bestFitsMat(:,3)+bestFitsMat(:,4))/2; % /2 for radius, another /2 for averaging
        
        %b='addison_benchmark_pupil.mat';%'addison_outdoor_pupil.mat';
%         b=strcat(name,'_',lighting,'_pupil.mat');
%         %2nd and 3rd vars
%         label=load(fullfile(labelPupildir,b));
%         gout=label.gout;
%         X=label.X;

        c=strcat(name,'_',lighting,'_raw.mat');
        %2nd and 3rd vars
        load(fullfile(datadir,c),'X');
        
        b=strcat(name,'_',lighting,'_raw_label.mat');
        %2nd and 3rd vars
        load(fullfile(labeldir,b),'pupilCentXMat','pupilCentYMat');
        gout=zeros(size(pupilCentXMat,1),2);
        gout(:,1)=pupilCentXMat;
        gout(:,2)=pupilCentYMat;
        
        %filename='addison_benchmark_pupil.mat';
        filename=strcat(name,'_',lighting,'_pupil.mat');
        
        save(fullfile(labelPupildir,filename),'-v7','X','gout','avgRadEllipse');
        
        
    end
end

fprintf('succeed')

%% Convert to V7- default
% no need to convert

% cd ~
% cd '/Users/ytun/Google Drive/IMPORTANT_VISION/MobiSys2015_labeling/labelPupil/'
% 
% %files={'kaituo','malai','sean'};
% clear;
% 
% load('addison_benchmark_pupil_v7.mat');
% save('addison_benchmark_pupil.mat','-v7');
