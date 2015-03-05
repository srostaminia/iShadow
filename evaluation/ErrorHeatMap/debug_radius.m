% 
% clear all;
% close all;
% 
% sub1=3;
% sub2=3;
% 
% data=load('/Users/ytun/iShadow/algorithms/cider/data/new/awesomeness_irb_radius_target_area_calFixed/addison_calib_pupil/results/subset_l1_init_strips_k7_lambda0.000100/cider_rep1.mat');
% 
% gout=data.gout;
% diff=data.radii.pred-data.radii.true;
% 
% figure;
% subplot(sub1,sub2,1);
% histogram(diff);
% 
% subplot(sub1,sub2,1);
% histogram(data.radii.pred);
% 
% subplot(sub1,sub2,1);
% histogram(data.radii.true);
% 
% [row,col]=round(gout);
% mat(row,col)=diff;
% 
% subplot(sub1,sub2,1);
% colormap('hot')
% imagesc(A)
% colorbar
% 
% subplot(sub1,sub2,1);
% 


%%

clear;
close all;

dataRootDir='/Users/ytun/iShadow/algorithms/cider/';

cd ~
addpath('/Users/ytun/iShadow/evaluation/ErrorHeatMap');
%addpath('/Users/yaminthuzartun/Desktop/Vision/yamin_data/addison2/full/results/subset_l1_init_strips_k7_lambda0.001000')
%subDir='data/awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results/subset_l1_init_strips_k7_lambda0.000464';%'awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results/subset_l1_init_strips_k7_lambda0.000100';

data=load('/Users/ytun/iShadow/algorithms/cider/data/new/awesomeness_irb_radius_target_mean/malai_calib_pupil/uniquefy_0/results/subset_l1_init_strips_k7_lambda0.000100/cider_rep1.mat');

filter=data.radii.true>0 & logical(sum(data.center.pred,2)) & data.radii.pred>0 & ~data.ann_used;
pos=data.center.true(filter,:);
dist=data.radii.pred(filter,:)-data.radii.true(filter,:);

diff=(data.radii.pred(filter,:)).^2-(data.radii.true(filter,:)).^2;
area=100*(diff)./(data.radii.true(filter,:)).^2;

sum((data.radii.true(filter,:))==0)
y=data.radii.true(filter,:);
sum(y>800)
median(area)
mean(area)
min(area)
max(area)
%mean(100*(abs(data.radii.pred(filter,:).^2-data.radii.true(filter,:).^2)./(data.radii.true(filter,:).^2)))

sub1=3;
sub2=3;
%--Plot heatmap for training data
figure;

subplot(sub1,sub2,1);
[errMat,dist]=createPupilErrorMat_radius(dist,pos);
colormap('hot');
imagesc(errMat);
colorbar;
title('Radius Error at Predicted Pupil Center');

subplot(sub1,sub2,2);
histogram(dist);
title('Radius Error distribution');

subplot(sub1,sub2,3);
histogram(area);
title('Radius Error distribution');
%%
% 
% 
% %%
% %--1. Blue: plot coordinates where training and test overlap 
% % check if training and test overlap(order matters: check test first)
% %Note:some errors that do not correspond to any coordinate in error matrix are set to 0.1
% inTestareinTrain= ( errMat_test~=-0.1 & errMat_train~=-0.1 );
% [y,x]=find(inTestareinTrain);
% %errMat_train(inTestareinTrain)<errMat_test(inTestareinTrain));
% 
% % a=[ -0.1 2; 1 6]
% % b=[1 1; -0.1 2]
% testErr=errMat_test;
% trainErr=errMat_train;
% testErr(testErr==-0.1 | trainErr==-0.1)=-0.1;
% trainErr(testErr==-0.1 | trainErr==-0.1)=-0.1;
% [y1,x1]=find(trainErr<testErr);
% 
% %--2. Green:plot coordinates where no training data exists, only test data exists
% [y2,x2]=find(errMat_test~=-0.1 & errMat_train==-0.1);
%  
% %--3. Magenta: plot coordinates where individual test error is greater than the mean test error
% testErr1=errMat_test;
% testErr1(errMat_test==-0.1)=0;  %to avoid -0.1 affecting comparison with mean
% 
% trainErr1=errMat_train;
% trainErr1(errMat_train==-0.1)=0;  %to avoid -0.1 affecting comparison with mean
% 
% mean_testErr=mean(dist_test);
% [y3,x3]=find(testErr1>mean_testErr);
% 
% %--4. Yellow:plot coordinates where individual test error is less than the mean test error
% [y4,x4]=find(testErr1~=0 & testErr1<mean_testErr);
% 
% %--5. Yellow:plot coordinates where individual test and training error is more than the overall mean error
% %err1_all=[trainErr1;testErr1];
% dist_all=[dist_train;dist_test];
% mean_allerr=mean(dist_all);
% 
% [y5,x5]=find(testErr1>mean_allerr); % | trainErr1>mean_allerr);
% 
% % size(errMat_test,1)*size(errMat_test,2)
% % size(y1)
% % size(y2)
% % size(y3)
% % size(y4)
% 
% figure;
% %HeatMap(mat);
% colormap('hot');
% imagesc(errMat_train);
% colorbar;
% hold on;
% 
% scatter(x,y,'ob');
% %scatter(x1,y1,'.y');
% %scatter(x2,y2,'xg');
% %scatter(x3,y3,'om');
% %scatter(x4,y4,'oy');
% 
% hold off;
% %legend('test but no training','testerror>mean test error')
% %legend('test error>train error','test but no training','testerror>mean test error','test error<mean test error')
% legend('overlap of test error and training error')
% title('Heat Map of Training Data with indicators');
% 
% % Test errors that are greater than the mean of test+training error
% [y6,x6]=find(errMat_test~=-0.1);  %Test data
% [y7,x7]=find(errMat_train~=-0.1); %Training data
% 
% 
% figure;
% %HeatMap(mat);
% colormap('hot');
% imagesc(errMat_train);
% colorbar;
% hold on;
% 
% scatter(x5,y5,'oy');  %error>mean error
% scatter(x6,y6,'xg');  %Test data
% %scatter(x7,y7,'xb');
% 
% hold off;
% legend('individual test error>mean overall error','test data')
% %legend('test error>train error','test but no training','testerror>mean test error','test error<mean test error')
% title('Heat Map of Overall Data with indicators');
% %%
% %rightskewed
% addpath('/Users/ytun/iShadow/algorithms/cider/data/awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results/subset_l1_init_strips_k7_lambda0.000100')
% 
% figure;
% %load('v7_rep1.mat')
% histfit([these_results.dist_test;these_results.dist_train])
% hold on
% scatter(median([these_results.dist_test;these_results.dist_train]),0,'xg')
% scatter(mean([these_results.dist_test;these_results.dist_train]),0,'xr')
% hold off
% title('histogram distribution of test+training eerros')
% 
% figure;
% %load('v7_rep1.mat')
% histfit(these_results.dist_test)
% hold on
% scatter(median(these_results.dist_test),0,'xg')
% scatter(mean(these_results.dist_test),0,'xr')
% hold off
% title('histogram distribution of test eerros')
% 
% figure;
% %load('v7_rep1.mat')
% histfit(these_results.dist_train)
% hold on
% scatter(median(these_results.dist_train),0,'xg')
% scatter(mean(these_results.dist_train),0,'xr')
% hold off
% title('histogram distribution of training eerros')
