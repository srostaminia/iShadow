% center_var	radius_var	center_fixed	radius_fixed
% 1.236313145	1.16335406	0.811168352	1.491844996
% 1.030699391	1.032674413	1.317487919	1.323816631
% 1.571429884	1.244248715	0.796882045	0.568146939
% 1.536330068	0.830678854	0.851410973	1.038732323
% 			
% 			
% center_var	radius_var	center_fixed	radius_fixed
% 0.279153469	0.155663329	0.143085909	0.105230921
% 0.292015174	0.147283473	0.196012316	0.188092407
% 0.373200018	0.197666247	0.106213458	0.0139726
% 0.448182124	0.23661873	0.106433621	0.062370234

%%


clear;
close all;

subList={'malai','mike','shuo','kaituo'};%,'addison','duncan'};%'addison',,'duncan'

nFrames_dark=repmat(1.96./(sqrt([1734;1841;2250;1875])),1,2);
nFrames_calib=repmat(1.96./(sqrt([5727;6008;6163;5879])),1,2);

nFrames=[nFrames_dark nFrames_calib];
dir='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err';
addpath('/Users/ytun/iShadow/evaluation/Error_graphs');

nUsers=5;%size(centErr_ann,1);

%% TABLE
%center_var	radius_var	center_fixed	radius_fixed
mat=[1.236313145	1.16335406	0.811168352	1.491844996;
    1.030699391	1.032674413	1.317487919	1.323816631;
    1.571429884	1.244248715	0.796882045	0.568146939;
    1.536330068	0.830678854	0.851410973	1.038732323];


std_mat=nFrames.*[0.279153469	0.155663329	0.143085909	0.105230921;
    0.292015174	0.147283473	0.196012316	0.188092407;
    0.373200018	0.197666247	0.106213458	0.0139726;
    0.448182124	0.23661873	0.106433621	0.062370234];

% xticklist=cell(1,length(annIndPerAllSub)+1);
% 
% for i=1:length(annIndPerAllSub)
%     xticklist{i}=sigfig(annIndPerAllSub(i),2);
% end
% xticklist{i+1}='100';

%ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];

%compare_barGraph(error2modelMat,subList,nLambda)
legendlist={'malai','mike','shuo','kaituo'};

%nUsers=length(subList);

% figure;
% bar_custom(mat(:,1:2),std_mat(:,1:2),xticklist,legendlist);%,upperErExist);
% 
% 
% 
% figure;
% bar_custom(mat(:,2:4),std_mat(:,2:4),xticklist,legendlist);%,upperErExist);


table_Var = table(mat(:,1),mat(:,2),std_mat(:,1),std_mat(:,2),...
    'VariableNames',{'center_err','size_err','center_std','size_std'});

table_Fixed = table(mat(:,3),mat(:,4),std_mat(:,3),std_mat(:,4),...
    'VariableNames',{'center_err','size_err','center_std','size_std'});

    writetable(table_Var,fullfile(dir,'pupilvar_sub.csv'));
    writetable(table_Fixed,fullfile(dir,'pupilfixed_sub.csv'));
