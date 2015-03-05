clear;
close all;

%subtract one to mean
%percentile
%scale
%1.96-95 confidence level

addpath('~/iShadow/evaluation/Error_graphs')
subList={'addison','duncan','kaituo','malai','mike','shuo'};
folderName='awesomeness_irb_radius_target_mean';%%irb_pupil';
rootDir=fullfile('~/iShadow/algorithms/cider/data/new',folderName);
addpath('~/iShadow/algorithms/cider/');

lightingSubFolder='_calib_pupil';%
uni='uniquefy_0';
objToPlotList={'pupilLoc-pixel','pupilLoc-deg','area','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
titleObjToPlotList={'pupil center error (pixels)','pupil center error (degrees)','pupil area percentage error','pupil radius absolute error'}; %pupilLoc_deg,pupilLoc_pixel,area,radius

unitList={'#pixels','degree','%','#pixels'};
saveOn=1;

graphDir='/Users/ytun/Google Drive/IMPORTANT_VISION/Graphs';
subpathToSave=fullfile(graphDir,folderName);
mkdir(subpathToSave);

subModelName='';%'rerun2_testind_';

linestylelist={'-','--','o-','*-'};

nUsers=length(subList);



uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
uni=uniFolderList{1};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

nLambda=length(lambdaStrList);
%errList=cell(1,length(subList));
nUni=length(uniFolderList);
annUsedStEachLambdaEachSub =struct;

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

totalFrameEachSub=zeros(nUsers,1);

%ori=pwd;
% cd (rootDir);
% subNameFolderCell=cellstr(strsplit(ls));
% cd(ori);
modelName='cider';

compareLine=0;

objToPlot='radius';

figure;

sub1=nUsers;
sub2=2;

for k=1%:nLambda
    
    lambdaFolder=lambdaFolderList{k};
    %lambdaStr=lambdaStrList{k};
    
    
    %dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));
    
    for i=1:nUsers
        
        subName=subList{i}; %subject name
        %indFile=i;
        subNameFolder=strcat(subName,lightingSubFolder);%subNameFolderCell{indFile};%
        
        for m=1%:nRep
            
            %cider_dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
            cider_dir= fullfile(rootDir,strcat(subNameFolder),'uniquefy_0/results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
            data=load(cider_dir);
            
            errMatEachSub=cell(nRep,nLambda); %errSt 5x10
            err_line_MatEachSub=cell(nRep,nLambda);
            annUsedMatEachSub=zeros(nRep,nLambda);
            indPerMatEachSub=zeros(nRep,nLambda);
            
            dir= fullfile(rootDir,strcat(subNameFolder),'uniquefy_0/results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));
            
            data=load(dir);
            
            %size(pi*((data.radii.true(data.radii.true>0)).^2))
            %size(areaTrue)
            rad_filter=data.radii.true>0;
            filter=logical(sum(data.center.pred,2)) & rad_filter;
            filter_line=filter & ~data.ann_used & rad_filter;
            
            if strcmp(objToPlot,'area')
                areaTrue=pi*(data.radii.true).^2;
                area_raw=100*(data.area.pred-areaTrue)./areaTrue;
                %errMatEachSub(m,k,j)=data.mean_model.area.diff_perc;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                
                %trueA=[areaTrue(filter) data.radii.true(filter) data.area.pred(filter) data.radii.pred(filter)];
                vect_raw=area_raw;
                true_raw=areaTrue;
            elseif strcmp(objToPlot,'radius')
                
                rad_raw=(data.radii.pred-data.radii.true);
                
                %abs(data.mean_model.radius.diff);%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                
                
                vect_raw=rad_raw;
                true_raw=data.radii.true;
            end
            
            vect_raw=(vect_raw(filter_line));
            true_raw=true_raw(filter_line);
            
            subplot(sub1,sub2,i*2-1);
            meann=mean(vect_raw);
            mediann=median(vect_raw);
            histogram(vect_raw);
            hold on;
            plot(meann,0,'xr','markersize',10);
            plot(mediann,0,'ob','markersize',10);
          
            hold off;
            title(sprintf('Error: %s, mean: %s',subName,num2str(meann,'%.2f')));
            
            subplot(sub1,sub2,i*2);
            histogram(true_raw);
            hold on;
            plot(mean(true_raw),0,'xr','markersize',10);
            plot(median(true_raw),0,'ob','markersize',10);

            hold off;
            title(sprintf('True value: %s, mean: %s',subName,num2str(mean(true_raw),'%.2f')));
            
            
        end
        
        
        
        
        
    end
    
    %errMatEachSub
    
end
