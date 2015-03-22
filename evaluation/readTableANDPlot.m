%% calib
clear;
close all;

pathToSave='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err';
plotMode='';
saveOn=1;

addpath(fullfile(pathToSave,'finalTable'));
MAT_centErr_ann_ideal=zeros(8,2);
MAT_power_ann_ideal=zeros(8,2);

[MAT_centErr_ann_ideal(:,1), MAT_power_ann_ideal(:,1)]=csvimport('ideal_calib_table_ANN.csv', 'columns',{'centErr_ann','power_ann'});%,'centErrCI_ann'});
%[MAT_centErr_ann_ideal(:,2), MAT_power_ann_ideal(:,2)]=csvimport('ideal_dark_table_ANN.csv', 'columns',{'centErr_ann','power_ann'});%,'centErrCI_ann'});

centErr_ann_ideal=mean(MAT_centErr_ann_ideal,2);
power_ann_ideal=mean(MAT_power_ann_ideal,2);

[centErr_cider_switching_calib, power_cider_switching_calib]=csvimport('switching_calib_table_Cider.csv', 'columns', {'centErr_cider','power_cider'});%,'centErrCI_cider'});
%[centErr_cider_switching_dark, power_cider_switching_dark] = csvimport('switching_dark_table_Cider.csv', 'columns', {'centErr_cider','power_cider'});%,'centErrCI_cider'});

figure;
%plot(centErr_ann_ideal,power_ann_ideal);
plot(centErr_cider_switching_calib,power_cider_switching_calib);
hold on;
%plot(centErr_cider_switching_dark,power_cider_switching_dark);

[centErr_cider_ideal_calib, power_cider_ideal_calib]=csvimport('ideal_calib_table_Cider.csv', 'columns', {'centErr_cider','power_cider'});
%[centErr_cider_ideal_dark, power_cider_ideal_dark]=csvimport('ideal_dark_table_Cider.csv', 'columns', {'centErr_cider','power_cider'});

plot(centErr_cider_ideal_calib,power_cider_ideal_calib,'bx');
%plot(centErr_cider_ideal_dark,power_cider_ideal_dark,'rx');

hold off;
ylabel('Power (W?)');
xlabel('Pupil Center Error (pixels)');
legend({'centErr_ann_ideal','centErr_cider_switching_calib','centErr_cider_switching_dark','centErr_cider_ideal_calib','centErr_cider_ideal_dark'});

pathToSave='';
plotFileName='';
plotFormatQ_final(pathToSave,plotFileName,saveOn,plotMode)



%% dark

% 
% 'ideal_dark_table_ANN.csv'
% 'ideal_dark_table_Cider.csv'
% 'switching_calib_table_Cider.csv'
% 'switching_dark_table_Cider.csv'
% 
% 
% 
% 
% M = csvread(filename)
