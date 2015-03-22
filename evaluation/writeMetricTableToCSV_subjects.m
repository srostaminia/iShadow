clear;
close all;

dir='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err';
addpath('/Users/ytun/iShadow/evaluation/Error_graphs');

nUsers=5;%size(centErr_ann,1);

%% TABLE
%center_var	radius_var	center_fixed	radius_fixed
mat=[1.236313145	1.16335406	0.811168352	1.491844996;
    1.030699391	1.032674413	1.317487919	1.323816631;
    1.571429884	1.244248715	0.796882045	0.568146939;
    1.536330068	0.830678854	0.851410973	1.038732323];


std_mat=[0.279153469	0.155663329	0.143085909	0.105230921;
    0.292015174	0.147283473	0.196012316	0.188092407;
    0.373200018	0.197666247	0.106213458	0.0139726;
    0.448182124	0.23661873	0.106433621	0.062370234]



bar_custom(mat(1:2),std_mat(1:2),xticklist,legendlist);%,upperErExist);


xticklist=cell(1,length(annIndPerAllSub)+1);

for i=1:length(annIndPerAllSub)
    xticklist{i}=sigfig(annIndPerAllSub(i),2);
end
xticklist{i+1}='100';

%ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];

%compare_barGraph(error2modelMat,subList,nLambda)

nUsers=length(subList);

figure;
upperErExist=0;
SObject=bar_custom(error2modelMat,std2modelMat,xticklist,legendlist);%,upperErExist);
ylabel(sprintf('Average Error (%s)',unitList{objInd}));
xlabel('ANN Size (Percent active pixels)');
title(sprintf('%s-%s: Average %s for %i different users',stage_list{mdInd},modelvsStrList{mdInd},titleObjToPlotList{objInd},nUsers),'FontSize', 11);
