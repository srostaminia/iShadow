%% graph without the points where predicted x,y are zeros

clear;
close all;

cd ~

addpath('~/iShadow/evaluation')
subList={'akshaya'};
rootDir='~/iShadow/algorithms/cider/data/';
nRep=5;

lambdaEnd=1;
modelName='cider';
radMarker=10;

uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

nLambda=length(lambdaStrList);
%errList=cell(1,length(subList));
nSub=length(subList);

figure;
for i=1:nSub
    
    subName=subList{i};
    %subName=subList{i}; %subject name
    
    
    
    for j=1:length(uniFolderList)
        
        uni=uniFolderList{j};
        
        for k=1:lambdaEnd%length(lambdaFolderList)
            
            lambdaFolder=lambdaFolderList{k};
            %lambdaStr=lambdaStrList{k};
            
            for m=1:nRep
                
                if strcmp(subName,'addison')
                    dir=fullfile(rootDir, 'awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results',lambdaFolder);
                    
                elseif strcmp(subName, 'yamin')
                    dir=fullfile(rootDir, 'awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results',lambdaFolder);
                    
                else
                    dir= fullfile(rootDir,'awesomeness_irb_pupil',strcat(subName,'_calib_pupil'),uni,'results',lambdaFolder);
                end
                
                fileName=strcat(modelName,'_rep',num2str(m),'.mat');
                
                
                %% plot direction
                data=load(fullfile(dir,fileName));
                
                %pred=%[6 4; 5 10; 1 2; 3 4];
                %true=%repmat([5 5],[size(pred,1) 1]);
                filter=logical(sum(data.pred,2));
                
                pred=data.pred;
                pred=pred(filter,:);
                gout=data.gout;
                gout=gout(filter,:);
                
                diff=pred-gout;
                
                subplot(nSub,nRep,(i-1)*nRep+m);
                
                hold on;
                scatter(diff(:,1),diff(:,2),radMarker,'or');
                scatter(0,0,2*radMarker,'xb');

                hold off;
                
                title(sprintf('sub=%s,reptition=%i',subName,m));
                
                
                
                
                halfDiff=1.25*max(abs(diff));
                xlim([-1*halfDiff(1) halfDiff(1)]);
                ylim([-1*halfDiff(2) halfDiff(2)]);
                
%                 %%
%                 fileErr=fullfile(dir,'v7_rep2.mat');
%                 
%                 %--Plot heatmap for training data
%                 figure;
%                 %subplot(1,1,1);
%                 dataSel='train';
%                 
%                 [errMat_train,dist_train]=createErrorMat(fileErr,dataSel);
%                 colormap('hot');
%                 imagesc(errMat_train);
%                 colorbar;
%                 title('Heat Map of Training Data');
%                 
%                 
%                 %--Plot heatmap for test data
%                 figure;
%                 %subplot(2,1,2);
%                 dataSel='test';
%                 [errMat_test,dist_test]=createErrorMat(filepath,dataSel);
%                 colormap('hot');
%                 imagesc(errMat_test);
%                 colorbar;
%                 title('Heat Map of Test Data');
                
            end
        end
        
    end
    
end

legend('predicted pupil center','true pupil center');

suptitle(sprintf('lambda=%s',lambdaStrList{k}));

    


