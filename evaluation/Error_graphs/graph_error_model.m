%function [errStEachSub,meanErrMatForAllSub,annUsedStEachLambdaEachSub,meanAnnUsedForAllLambdaEachSub]=graph_error_models(subList, rootDir, modelName)

%clear
%addpath('~/iShadow/algorithms/cider/algorithms/cider')
%author- yamin
%subList={'yamin','addison','akshaya','duncan','mike'};
%rootDir='~/iShadow/algorithms/cider/rep/';
%modelName='cider' % 'ANN' %'both'

%graph uniquefy0 and uniquefy1 separately->unique
%higher lambda->more regularization/restriction->smaller subsampling
%***average for each users




function [errStEachSub,meanErrAllSub,stdErrAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model(modelName,subList,rootDir,object,objForTitle,ylabelunit)
cd ~

%lighting='outdoors';

uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

nLambda=length(lambdaStrList);
%errList=cell(1,length(subList));

annUsedStEachLambdaEachSub =struct;

errStEachSub = struct;
err_line_StEachSub = struct;
%.raw.uniquefy0.yamin=?each subject field 5x10
%.mean.uniquefy0.yamin=? each subject field 1x10
%.std.uniquefy0.yamin=? each subject field 1x10

%errStAllSub=struct; %over 6 users
%.raw.uniquefy0=?each subject field usersx10
%.mean.uniquefy0=? each subject field 1x10
%.std.uniquefy0=? each subject field 1x10

nUsers=length(subList);
nRep=5;

%fieldList = unique(subList);
%annFreq=zeros(1,size(subList));

totalFrameEachSub=zeros(nUsers,1);

for i=1:nUsers
    
    subName=subList{i};
    %subName=subList{i}; %subject name
    errMatEachSub=zeros(nRep,nLambda); %errSt 5x10
    err_line_MatEachSub=zeros(nRep,nLambda);
    annUsedMatEachSub=zeros(nRep,nLambda);
    
    subName
    
    for j=1:length(uniFolderList)
        
        uni=uniFolderList{j};
        
        for k=1:nLambda
            
            lambdaFolder=lambdaFolderList{k};
            %lambdaStr=lambdaStrList{k};
            
            for m=1:nRep
                
                % if strcmp(subName,'addison')
                % dir=fullfile(rootDir, 'awesomeness_pupil_microbench1_uniquefy/addison_microbench1_pupil/full/results',lambdaFolder,strcat(modelName,'_rep',num2str(m),'.mat'));
                %  dir= fullfile(rootDir,strcat('awesomeness_irb_',object),strcat(subName,'_','benchmark',object),uni,'results',lambdaFolder,strcat(modelName,'_rep',num2str(m),'.mat'));
                
                %elseif strcmp(subName, 'yamin')
                %  dir=fullfile(rootDir, 'awesomeness_pupil_1sm4_uniquefy/yamin_1sm4_pupil/full/results',lambdaFolder,strcat(modelName,'_rep',num2str(m),'.mat'));
                
                %else
                if strcmp(subName,'addison')
                    lighting='benchmark';
                else
                    lighting='calib';
                end
                
                subFolderName=strcat(subName,'_',lighting,'_',object);
                
                %                 i
                %                 m
                %                 k
                %                 j
                %                 subName
                %                 rootDir
                %                 strcat('awesomeness_irb_',object)
                %                 strcat(subName,'_',lighting,object)
                %                 uni
                %                 lambdaFolder
                %                 strcat(modelName,'_rep',num2str(m),'.mat')
                
                repdir= fullfile(rootDir,strcat('awesomeness_irb_',object),strcat(subName,'_',lighting,'_',object),uni,'results',lambdaFolder,strcat(modelName,'_rep',num2str(m),'.mat'));
                data=load(fullfile(rootDir,strcat(subFolderName,'.mat')));%'eye_rep_addison_microbench1_pupil_auto.mat'));
                % end
                
                %dir
                %                     if m==5 && strcmp(subName,'shuo')
                %
                %                     else
                %repdir
                rep=load(repdir);
                %rep
                
                
                
                repdir
         %       bool=rep.radii>0 & data.avgRadEllipse>0;
%                 max(rep.radii(bool))
%                 min(rep.radii(bool))
%                 max(data.avgRadEllipse(bool))
%                                 min(data.avgRadEllipse(bool))

%                 circRad=(rep.radii(bool)).^2;
%                 ellRad=(data.avgRadEllipse(bool)).^2;
%                 diffRatio=abs(circRad-ellRad)./ellRad;
%                 meandiff=mean(diffRatio);
%                 mean_areadiff=100*meandiff
                %errMatEachSub(m,k,j)=100*abs(mean(rep.radii(rep.radii>0)).^2-(mean(rep.avgRadEllipse(rep.avgRadEllipse>0)).^2))./mean(rep.avgRadEllipse(rep.avgRadEllipse>0)).^2;
                %size(errMatEachSub(m,k,j))
                %size(mean_areadiff)
             %   errMatEachSub(m,k,j)=%mean_areadiff;%100*mean_raddiff/((data.avgRadEllipse(bool)).^2)
                
                % mean_areadiff
                                    filter_Line=sum(data.gout,2)>-2;
size(data.gout)
                                    size(filter_Line)
                                    size(rep.err)
                errMatEachSub(m,k,j)=rep.err(filter_Line);
                
                
                if strcmp(modelName,'cider')
                    
                   % filter_Line = logical(sum(rep.pred(bool),2)) & ~rep.ann_used(bool);
                    
                    %                      size(filter_Line)
                    %                      size(diffRatio)
                    %                      size(diffRatio(filter_Line))
                    
                  %  err_line_MatEachSub(m,k,j)=100*mean(diffRatio(filter_Line));
                    
                    err_line_MatEachSub(m,k,j)=rep.err_Line(filter_Line);
                    annUsedMatEachSub(m,k,j)=sum(rep.ann_used);
                    
                end
                
            end
        end
        
    end
    
    
    
    
    
    
    %annUsedMatEachSub
    %mean(annUsedMatEachSub,1)
    
    if strcmp(modelName,'cider')
        annUsedStEachLambdaEachSub.raw.(uni).(subName)=annUsedMatEachSub; %5x10
        annUsedStEachLambdaEachSub.mean.(uni).(subName)=round(mean(annUsedMatEachSub,1));
        annUsedStEachLambdaEachSub.std.(uni).(subName)=std(annUsedMatEachSub,1);
        
        fprintf('ANNused1');
    end
    
    
    
    errStEachSub.raw.(uni).(subName)=errMatEachSub;
    errStEachSub.mean.(uni).(subName)=mean(errMatEachSub,1);
    %         size(errStEachSub.mean.(uni).(subName))
    %         size(errStEachSub.raw.(uni).(subName))
    errStEachSub.std.(uni).(subName)=std(errMatEachSub,1);
    
    
    err_line_StEachSub.raw.(uni).(subName)=err_line_MatEachSub;
    err_line_StEachSub.mean.(uni).(subName)=mean(err_line_MatEachSub,1);
    %         size(errStEachSub.mean.(uni).(subName))
    %         size(errStEachSub.raw.(uni).(subName))
    err_line_StEachSub.std.(uni).(subName)=std(err_line_MatEachSub,1);
end

if strcmp(modelName,'cider')
    totalFrameEachSub(i,:)=length(rep.ann_used);
    
    
    %fprintf('annused')
    %size(rep.ann_used)
    %size(rep.gout,1)
    if size(rep.ann_used,1)~=size(rep.gout,1)
        fprintf('ERROR: totalFrame is wrong. Check dimension\n\n');
        
    end
    
end



%compute mean for all subjects
meanErrMatForAllSub=zeros(nUsers,nLambda);
meanErrMat_line_ForAllSub=zeros(nUsers,nLambda);

for i=1:nUsers
    subName=subList{i};
    meanErrMatForAllSub(i,:)=errStEachSub.mean.uniquefy_0.(subName);
    meanErrMat_line_ForAllSub(i,:)=err_line_StEachSub.mean.uniquefy_0.(subName);
end


if strcmp(modelName,'cider')
    meanAnnUsedForEachLambdaEachSub=zeros(nUsers,nLambda);
    
    for i=1:nUsers %nUsers x nLambda: 6x10
        subName=subList{i};
        meanAnnUsedForEachLambdaEachSub(i,:)=annUsedStEachLambdaEachSub.mean.uniquefy_0.(subName);
        stdAnnUsedForEachLambdaEachSub(i,:)=annUsedStEachLambdaEachSub.std.uniquefy_0.(subName);
        
    end
    
end
%size(meanAnnUsedForEachLambdaEachSub)
%meanAnnUsedForEachLambdaEachSub

%% CIDER: Graph Subplot individual user's error
figure;
% uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
% lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
% lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
% lambdaStrList=fliplr(lambdaStrList);
% lambdaFolderList=fliplr(lambdaFolderList);

%nLambda=length(lambdaStrList);

uni='uniquefy_0'%%%f

for i=1:nUsers
    
    subName=subList{i};
    
    subplot(2,3,i);
    errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName));
    ylim([0, max(errStEachSub.mean.(uni).(subName))+max(errStEachSub.std.(uni).(subName))+0.1])
    xlim([0 nLambda+1])
    ylabel(ylabelunit);
    xlabel('ANN size (incrementing)');
    title(strcat('subject: ',subName));
    
end

suptitle({strcat(modelName,': Error of predicted ',objForTitle,' for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});

%% CIDER: One Graph Subplot individual user's error

figure;
for i=1:nUsers
    
    subName=subList{i};
    errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName));
    %meanErrMatForAllSub(i,:)=errStEachSub.mean.uniquefy_0.(subName);
    hold on;
end

hold off;
%ylim([0, 2.9]) %10.1
xlim([0 nLambda+1])
ylabel(ylabelunit);
xlabel('ANN size (incrementing)');
%title(strcat('subject: ',subName));
legend(subList);

suptitle({strcat(modelName,': Error of predicted ',objForTitle,' for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});

%% CIDER: AVG Graph over all user's errors

meanErrAllSub=mean(meanErrMatForAllSub,1);
stdErrAllSub=std(meanErrMatForAllSub,1);

figure;
size(stdErrAllSub)
size(meanErrAllSub)
size(stdErrAllSub)
errorbar(1:1:nLambda,meanErrAllSub,stdErrAllSub);
ylim([0 max(meanErrAllSub+stdErrAllSub+0.5)])
xlim([0 nLambda+1])
ylabel(strcat('Average ', ylabelunit));
xlabel('ANN size (incrementing)');

suptitle({strcat(modelName,': Average error of predicted ',objForTitle,' for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});





%% Line model CIDER: Graph Subplot individual user's error

if strcmp(modelName,'cider')
    modelNameL='lineCider'
    
    figure;
    % uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
    % lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
    % lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
    % lambdaStrList=fliplr(lambdaStrList);
    % lambdaFolderList=fliplr(lambdaFolderList);
    
    %nLambda=length(lambdaStrList);
    
    uni='uniquefy_0'%%%f
    
    for i=1:nUsers
        
        subName=subList{i};
        
        subplot(2,3,i);
        errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName));
        ylim([0, max(err_line_StEachSub.mean.(uni).(subName))+max(err_line_StEachSub.std.(uni).(subName))+0.1])
        xlim([0 nLambda+1])
        ylabel(ylabelunit);
        xlabel('ANN size (incrementing)');
        title(strcat('subject: ',subName));
        
    end
    
    suptitle({strcat(modelNameL,': Error of predicted ',objForTitle,' for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
    
    %% Line CIDER: One Graph Subplot individual user's error
    
    figure;
    for i=1:nUsers
        
        subName=subList{i};
        errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName));
        %meanErrMatForAllSub(i,:)=err_line_StEachSub.mean.uniquefy_0.(subName);
        hold on;
    end
    
    hold off;
    %ylim([0, 2.9]) %10.1
    xlim([0 nLambda+1])
    ylabel(ylabelunit);
    xlabel('ANN size (incrementing)');
    %title(strcat('subject: ',subName));
    legend(subList);
    
    suptitle({strcat(modelNameL,': Error of predicted ',objForTitle,' for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
    
    %% Line CIDER: AVG Graph over all user's errors
    
    meanErr_line_ForAllSub=mean(meanErrMat_line_ForAllSub,1);
    stdErr_line_ForAllSub=std(meanErrMat_line_ForAllSub,1);
    
    figure;
    errorbar(1:1:nLambda,meanErr_line_ForAllSub,stdErr_line_ForAllSub);
    ylim([0 max(meanErr_line_ForAllSub+stdErr_line_ForAllSub+0.5)])
    xlim([0 nLambda+1])
    ylabel(strcat('Average ', ylabelunit));
    xlabel('ANN size (incrementing)');
    
    suptitle({strcat(modelNameL,': Average error of predicted ',objForTitle,' for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});
    
else
    meanErr_line_ForAllSub=0;
    stdErr_line_ForAllSub=0;
end

%% CIDER: AVG Graph over all user's errors
%meanAnnUsedForEachLambdaEachSub

if strcmp(modelName,'cider')
    
    fprintf('ANNused');
    
    %         size(totalFrameEachSub)
    %         size(meanAnnUsedForEachLambdaEachSub)
    
   % size(meanAnnUsedForEachLambdaEachSub,2)
   % size(std(meanAnnUsedForEachLambdaEachSub,2))
   % size(meanAnnUsedForEachLambdaEachSub)
    meanAnnUsedForAllLambdaEachSub=100*(round(mean(meanAnnUsedForEachLambdaEachSub,2))./totalFrameEachSub);
    %stdAnnUsedForAllLambdaEachSub=100*(round(std(meanAnnUsedForEachLambdaEachSub,2))./totalFrameEachSub);
    meanAnnUsedForAllLambdaEachSub
    %annUsedStdForAllLambdaEachSub=std(meanAnnUsedForEachLambdaEachSub,2)
    
    totalFrameEachSub
    %        meanAnnUsedForAllLambdaEachSub
    %size(meanAnnUsedForAllLambdaEachSub)
    
    figure;
    bar(meanAnnUsedForAllLambdaEachSub)
    %errorb(meanAnnUsedForAllLambdaEachSub,stdAnnUsedForAllLambdaEachSub);
    set(gca, 'XTick',1:nUsers, 'XTickLabel',subList)
    ylabel('% of annUsedFreq');
    suptitle({strcat(modelName,': Average number of times ANN is being used in CIDER model for different users')});
    
end

%     %% CIDER: error and ann_used- v1
%     if strcmp(modelName,'cider')
%
%         meanAnnUsedForEachLambdaEachSub
%         errStEachSub.mean.(uni).(subName)
%
%
%         size(meanAnnUsedForEachLambdaEachSub)
%         size(errStEachSub.mean.(uni).(subName))
%
%         figure;
%
%         maxAnnUsed=max(meanAnnUsedForEachLambdaEachSub,[],1)
%
%         for k=1:nUsers
%
%             subName=subList{k};
%             subName
%
%             errStEachSub.mean.(uni).(subName)
%
%             meanAnnUsedVect=meanAnnUsedForEachLambdaEachSub(k,:);%./maxAnnUsed;
%
%             subplot(2,3,k);
%
%             bar(1:1:nLambda,[errStEachSub.mean.(uni).(subName)' meanAnnUsedVect']);
%             xlim([0 nLambda+1])
%             ylabel('% of annUsedFreq');
%             suptitle({strcat(modelName,': AvgANNused and mean error for ',subName)});
%
%         end
%     end
%
%
%
%     %% CIDER: error and ann_used -v2
%
%     if strcmp(modelName,'cider')
%
%         meanAnnUsedForEachLambdaEachSub
%         errStEachSub.mean.(uni).(subName)
%
%
%         size(meanAnnUsedForEachLambdaEachSub)
%         size(errStEachSub.mean.(uni).(subName))
%
%         figure;
%
%         maxAnnUsed=max(meanAnnUsedForEachLambdaEachSub,[],1)
%
%         meanErrVect_AllSub=zeros(nUsers,1);
%
%         for k=1:nUsers
%
%             subName=subList{k};
%             subName
%
%             %1x10
%
%             %3 is just scaling to equalize with meanAnnUsedVect in bargraph
%             meanErrVect_AllSub(k)=3*mean(errStEachSub.mean.(uni).(subName));
%
%
%
%         end
%
%         meanAnnUsedVect=meanAnnUsedForAllLambdaEachSub;
%
%         meanAnnUsedVect
%         meanErrVect_AllSub
%
%         bar(1:1:nUsers,[meanErrVect_AllSub meanAnnUsedVect]);
%         set(gca, 'XTick',1:nUsers, 'XTickLabel',subList)
%         ylabel('% of annUsedFreq and error %??');
%         legend('error%??','meanAnnUsedVect');
%         suptitle({strcat(modelName,': AvgANNused and mean error for different users')});
%
%
%     end

end
