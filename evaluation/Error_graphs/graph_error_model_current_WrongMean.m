%stack all error
%Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'
function [errStEachSub,err_line_StEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanlineUsedForAllSub,stdlineUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(saveOn,indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName,compareLine)
%function [errStEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanlineUsedForAllSub,stdlineUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName)

lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

repList={'rep1','rep2','rep3','rep4','rep5'};

nLambda=length(lambdaStrList);
%errList=cell(1,length(subList));
nUni=length(uniFolderList);

lineUsedStEachLambdaEachSub =struct;
errStEachSub = struct;
err_line_StEachSub = struct;
indStEachSub = struct;
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

totalFrameEachSub=zeros(nUsers,1); %INCLUDE ALL REPS for each user

%ori=pwd;
% cd (rootDir);
% subNameFolderCell=cellstr(strsplit(ls));
% cd(ori);
sizeVectForDiffLambda=zeros(5,nUsers,nLambda);

count=0;

for k=1:nLambda
    
    lambdaFolder=lambdaFolderList{k};
    %lambdaStr=lambdaStrList{k};
    
    nFramesForEachLambda=0;
    
    for i=1:nUsers
        
        subName=subList{i}; %subject name
        subNameFolder=strcat(subName,lightingSubFolder);%subNameFolderCell{indFile};%
        
        for m=1:nRep
            repi=repList{m};
            
            
            
            dir= fullfile(rootDir,strcat(subNameFolder),'results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));            
            data=load(dir);
            
            if compareLine && strcmp(modelName,'ann')
                cider_dir= fullfile(rootDir,strcat(subNameFolder),'results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
                % cider_dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
                cider_data=load(cider_dir);
            end
            
            %size(pi*((data.radii.true(data.radii.true>0)).^2))
            %size(areaTrue)
            if strcmp(objToPlot,'pupilLoc-pixel')
                rawData=data.center.diff.raw;
            elseif strcmp(objToPlot,'radius')
                rawData=data.radii.diff;
            end
            
            if strcmp(modelName,'cider')
                err_line_StEachSub.raw.(subName).(repi)=rawData(data.filter_Line);%data.err_Line;%
                lineUsedStEachLambdaEachSub.raw.(subName).(repi)=data.perc_LineUsed;
            elseif strcmp(modelName,'ann') && compareLine
                rawData=rawData(cider_data.filter_Line);%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
            end
            
            errStEachSub.raw.(subName).(repi)=rawData;
            indStEachSub.raw.(subName).(repi)=data.indActivePercent;
            
            sizeVectForDiffLambda(m,i,k)=size(rawData,1);
            
        end
       
    end
            
end


err_Lambdas_mean=zeros(nLambda,1); %errSt 5x10
indPer_Lambdas_mean=zeros(nLambda,1);

if strcmp(modelName,'cider')
    errLine_Lambdas_mean=zeros(nLambda,1);
    lineUsed_Lambdas_mean=zeros(nLambda,1);
end

%concatenate
for k=1:nLambda
    err_temp=zeros(sizeVectForDiffLambda(k),1);
    indPer_temp=zeros(5*nUsers,1);
    
    if strcmp(modelName,'cider')
        errLine_temp=zeros(sizeVectForDiffLambda(k),1);
        lineUsed_temp=zeros(5*nUsers,1);
    end

    for i=1:nUsers
        subName=subList{i}; %subject name
        
        for m=1:nRep
            repi=repList{m};
            
            if strcmp(modelName,'cider')
                err_temp()=err_line_StEachSub.raw.(subName).(repi);%data.err_Line;%
                lineUsedStEachLambdaEachSub.raw.(subName).(repi);
             end
            
            errStEachSub.raw.(subName).(repi)=rawData;
            indStEachSub.raw.(subName).(repi)=data.indActivePercent;
            
            nFramesForEachLambda=nFramesForEachLambda+size(rawData,1);
            
        end
       
    end
    
    sizeVectForDiffLambda(k)=nFramesForEachLambda;
        
end 
    
    

%compute mean for all subjects
meanErrMatForAllSub=zeros(nUsers,nLambda);
meanErrMat_line_ForAllSub=zeros(nUsers,nLambda);
indPerMatForAllSub=zeros(nUsers,nLambda);
meanlineUsedMatForAllSub=zeros(nUsers,nLambda);
meanLineUsedMatForAllSub=zeros(nUsers,nLambda);

for i=1:nUsers
    subName=subList{i};
    meanErrMatForAllSub(i,:)=errStEachSub.raw.(subName);
    indPerMatForAllSub(i,:)=indStEachSub.raw.(subName);
end

save('meanErrMatForAllSub_6subs.mat','meanErrMatForAllSub');

if strcmp(modelName,'cider')
    %stdlineUsedForAllSub=zeros(nUsers,nLambda);
    
    for i=1:nUsers %nUsers x nLambda: 6x10
        subName=subList{i};
        meanlineUsedMatForAllSub(i,:)=lineUsedStEachLambdaEachSub.mean.(subName);
        meanLineUsedMatForAllSub(i,:)=100-lineUsedStEachLambdaEachSub.mean.(subName);
        
        meanErrMat_line_ForAllSub(i,:)=err_line_StEachSub.mean.(subName);
        
        %         FreqlineUsedMatForAllSub(i,:)=lineUsedStEachLambdaEachSub.mean.(subName);
        %         FreqLineUsedMatForAllSub(i,:)=100-lineUsedStEachLambdaEachSub.mean.(subName);
        
    end
    
end
%% CIDER: Graph Subplot individual user's error
%figure;


%save(fullfile('annSize.mat'),'indPerMatForAllSub');



for i=1:nUsers
    
    subName=subList{i};
    errorbar(1:1:nLambda,meanErrMatForAllSub(i,:),errStEachSub.std.(subName),linestyle,'markers',10);
    hold on;
end

% hold off;
% limsy=get(gca,'YLim');
% set(gca,'Ylim',[0 limsy(2)]);
% %ylim([0, 2.9]) %10.1
% xlim([0 nLambda+1])
% ylabel('Error (pixels)');
% xlabel('ANN size (incrementing)');
% legend(subList);
%
% suptitle(sprintf('%s: Error for %i different users',modelName,nUsers));
% pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);

%% CIDER: AVG Graph over all user's errors

meanErrAllSub=mean(meanErrMatForAllSub,1);
stdErrAllSub=std(meanErrMatForAllSub,0,1);
indPerForAllSub=mean(indPerMatForAllSub,1);

if strcmp(modelName,'cider')
    
    meanErr_line_ForAllSub=mean(meanErrMat_line_ForAllSub,1);
    stdErr_line_ForAllSub=std(meanErrMat_line_ForAllSub,0,1);
    
    meanlineUsedForAllSub=round(mean(meanlineUsedMatForAllSub,1));
    stdlineUsedForAllSub=(std(meanlineUsedMatForAllSub,0,1));
    
    meanLineUsedForAllSub=round(mean(meanLineUsedMatForAllSub,1));
    stdLineUsedForAllSub=(std(meanLineUsedMatForAllSub,0,1));
else
    meanErr_line_ForAllSub=0;
    stdErr_line_ForAllSub=0;
    
    meanlineUsedForAllSub=0;
    stdlineUsedForAllSub=0;
    
    meanLineUsedForAllSub=0;
    stdLineUsedForAllSub=0;
end

if saveOn
    save();
    fprintf('Data SAVED!');
end

end

%inside strcmp(--'target')
% figure;
% subplot(2,2,1);
% histogram(100*mat2gray(data.center.diff.raw.raw((data.filter & ~data.ann_used),:)));
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% title('line model: error distribution')
% subplot(2,2,2);
% histogram(100*mat2gray(data.center.diff.raw.raw(data.filter,:)));
% title('cider model: error distribution')
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% subplot(2,2,3);
% histogram(100*mat2gray(data.center.diff.raw.raw((data.filter & data.ann_used),:)));
% title('ann model: error distribution')
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% suptitle('Pupil Center Error: All data including training data')
% figure;

