%stack all error
%Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'
function graph_error_model_current(modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName,compareLine)
%function [errStEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanlineUsedForAllSub,stdlineUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName)
 

lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

repList={'rep1','rep2','rep3','rep4','rep5'};

nLambda=length(lambdaStrList);

lineUsedStEachLambdaEachSub =struct;
errStEachSub = struct;
err_line_StEachSub = struct;
indStEachSub = struct;
%.uniquefy0.yamin=?each subject field 5x10
%.mean.uniquefy0.yamin=? each subject field 1x10
%.std.uniquefy0.yamin=? each subject field 1x10

%errStAllSub=struct; %over 6 users
%.uniquefy0=?each subject field usersx10
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

if strcmp(modelName,'cider')
sizeVectForDiffLambda_line=zeros(5,nUsers,nLambda);
end

count=0;

for k=1:nLambda
    
    lambdaFolder=lambdaFolderList{k};
    %lambdaStr=strcat('lambda',num2str(k));
    
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
                err_line_StEachSub.(subName).(repi)=rawData(data.filter_Line);%data.err_Line;%
                lineUsedStEachLambdaEachSub.(subName).(repi)=data.perc_LineUsed;
                sizeVectForDiffLambda_line(m,i,k)=size(rawData,1);
            elseif strcmp(modelName,'ann') && compareLine
                rawData=rawData(cider_data.filter_Line);%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
            end
            
            errStEachSub.(subName).(repi)=rawData;
            indStEachSub.(subName).(repi)=data.indActivePercent;
            
            sizeVectForDiffLambda(m,i,k)=size(rawData,1);
            
        end
       
    end
            
end


combo=struct;

%concatenate
for k=1:nLambda
    err_temp=zeros(sizeVectForDiffLambda(k),1);
    indPer_temp=zeros(5*nUsers,1);
    
    if strcmp(modelName,'cider')
        combo.(strcat('lambda',num2str(k))).errLine_temp=zeros(sizeVectForDiffLambda_line(k),1);
        combo.(strcat('lambda',num2str(k))).lineUsed_temp=zeros(5*nUsers,1);
    end

    for i=1:nUsers
        subName=subList{i}; %subject name
        
        for m=1:nRep
            repi=repList{m};
            
            if i==1 && m==1
                count=1;
            end
                
            if strcmp(modelName,'cider')
                combo.(strcat('lambda',num2str(k))).errLine_temp(count:count+sizeVectForDiffLambda_line(m,i,k)-1)=err_line_StEachSub.(subName).(repi);%data.err_Line;%
                combo.(strcat('lambda',num2str(k))).lineUsed_temp((i-1)*5+m)=lineUsedStEachLambdaEachSub.(subName).(repi);%data.err_Line;

            end
            
            combo.(strcat('lambda',num2str(k))).err_temp(count:count+sizeVectForDiffLambda(m,i,k)-1)=errStEachSub.(subName).(repi);%data.err_Line;%
            combo.(strcat('lambda',num2str(k))).indPer_temp((i-1)*5+m)=indStEachSub.(subName).(repi);%data.err_Line;%
            
        end
       
    end
            
end 
    

%compute mean for all subjects

err_Lambdas_mean=zeros(nLambda,3); %errSt 5x10
indPer_Lambdas_mean=zeros(nLambda,1);

for k=1:nLambda
    
    err_Lambdas_mean(k,2)=mean(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).err_temp)));
    indPer_Lambdas_mean(k)=mean(combo.(strcat('lambda',num2str(k))).indPer_temp);
    
    s=length(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).err_temp)));
    err_Lambdas_mean(k,3)=1.96*(1/sqrt(s))*std(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).err_temp)));
    %indPer_Lambdas_mean(k,3)=1.96*(1/sqrt(length(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).indPer_temp)))*mean(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).indPer_temp);
end


if strcmp(modelName,'cider')
    errLine_Lambdas_mean=zeros(nLambda,3);
    lineUsed_Lambdas_mean=zeros(nLambda,3);
    
    errLine_Lambdas_mean(:,1)=zeros(nLambda,3);
    lineUsed_Lambdas_mean(:,1)=zeros(nLambda,3);
    
    for k=1:nLambda
        
        errLine_Lambdas_mean(k,2)=mean(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).errLine_temp)));
        %lineUsed_Lambdas_mean(k,2)=mean(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).lineUsed_temp)));
        
        s=length(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).errLine_temp)));
        errLine_Lambdas_mean(k,3)=1.96*(1/sqrt(s))*std(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).errLine_temp)));
        %lineUsed_Lambdas_mean(k,3)=1.96*(1/sqrt(length(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).lineUsed_temp)))*std(cell2mat(struct2cell(combo.(strcat('lambda',num2str(k))).lineUsed_temp))));
        
    end
end

err_Lambdas_mean(:,1)=indPer_Lambdas_mean;
errLine_Lambdas_mean(:,1)=indPer_Lambdas_mean;
%lineUsed_mean=mean(lineUsed_Lambdas_mean);

dir='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err';
save(fullfile(dir,strcat(objToPlot,'Err_',modelName,'_',subModelName,'.mat')),'err_Lambdas_mean','errLine_Lambdas_mean');%,%'lineUsed_Lambdas_mean');
 