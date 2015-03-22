clear;
close all;

dir='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err/calib/old';

addpath('/Users/ytun/iShadow/evaluation/Error_graphs');

%% TABLE
nUsers=6;%size(centErr_ann,1);

%6.1. Stages
fileNameANN_CENT={'pupilLoc-pixelErr_ann__IdealLine.mat','pupilLoc-pixelErr_ann__lineCmpr.mat'};
fileNameANN_RAD={'radiusErr_ann__IdealLine.mat','radiusErr_ann__lineCmpr.mat'};
fileNameCider_CENT={'pupilLoc-pixelErr_cider_idealLINE100Test__IdealLine.mat','pupilLoc-pixelErr_cider__lineCmpr.mat'};
fileNameCider_RAD={'radiusErr_cider_idealLINE100Test__IdealLine.mat','radiusErr_cider__lineCmpr.mat'};

lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};

load(fullfile(dir,'../..','frequency.mat'));
load(fullfile(dir,'../..','table_power.mat'));
%power_data_ann.sampling_cost_pixels

fileNameList={'stage_','switching_'};

accuracyList={'Pupil Center', 'Pupil Size'};
metricList={'NIR Cost','Sampling Power Cost','Computation Power Cost','Speed','Sampling Cost (pixels)'};

sub1=2; sub2= 5;

savePlotOn=1;
saveTableOn=1;
plotOn=1;

ErrIdealCross_Cell={};

if saveTableOn
   mkdir(fullfile(dir,'table')); 
end

for i=1:2
    %ROWS
    %CentErrCI=
    %sizeErrCI=
    
    SampCostPixels_ann=100*power_data_ann.sampling_cost_pixels(:,1:8)/112^2;
    SampCostPower_ann=power_data_ann.sampling_cost_power;
    CompCost_cycles_ann=power_data_ann.computation_cost_cycles;
    CompCost_power_ann=power_data_ann.computation_cost_power;
    Speed_ann=power_data_ann.framerate;
    NIRCost_ann=power_data_ann.nir_cost;
    
    disp(i);
    if i==2 %6.2 Switching
        
        %COLS
        data=load(fullfile(dir,fileNameANN_CENT{i}));
        centErr_ann=mean(data.meanErrMatForAllSub(:,1:8),1);
        centErrCI_ann=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        
        
        
        data=load(fullfile(dir,fileNameANN_RAD{i}));
        sizeErr_ann=mean(data.meanErrMatForAllSub(:,1:8),1);
        sizeErrCI_ann=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        
        data=load(fullfile(dir,fileNameCider_CENT{i}));
        centErr=mean(data.meanErrMatForAllSub(:,1:8),1);
        centErrCI=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        
        data=load(fullfile(dir,fileNameCider_RAD{i}));
        sizeErr=mean(data.meanErrMatForAllSub(:,1:8),1);
        sizeErrCI=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        
        SampCostPixels=100*power_data_cider.sampling_cost_pixels/112^2;
        SampCostPower=power_data_cider.sampling_cost_power;
        CompCost_cycles=power_data_cider.computation_cost_cycles;
        CompCost_power=power_data_cider.computation_cost_power;
        Speed=power_data_cider.framerate;
        NIRCost=power_data_cider.nir_cost;
        
        ANNUsedPerc=mean(FreqAnnUsedMatForAllSub(:,1:8),1);
        CiderUsedPerc=mean(FreqLineUsedMatForAllSub(:,1:8),1);
        
        table_ANN = table(ANNUsedPerc',centErr_ann',sizeErr_ann',centErrCI_ann',sizeErrCI_ann',NIRCost_ann',SampCostPower_ann',CompCost_cycles_ann',CompCost_power_ann',Speed_ann',SampCostPixels_ann','VariableNames',{'ANNUsedPerc','centErr_ann','centErrCI_ann','sizeErr_ann','sizeErrCI_ann','NIRCost_ann','SampCostPower_ann','CompCost_cycles_ann','CompCost_power_ann','Speed_ann','SampCostPixels_ann'});
        table_Cider = table(CiderUsedPerc',centErr',centErrCI',sizeErr',sizeErrCI',NIRCost',SampCostPower',CompCost_cycles',CompCost_power',Speed','VariableNames',{'CiderUsedPerc','centErr','centErrCI','sizeErr','sizeErrCI','NIRCost','SampCostPower','CompCost_cycles','CompCost_power','Speed'});
        
        
    else %6.1 Stage comparison
        data=load(fullfile(dir,fileNameANN_CENT{i}));
        centErr_ann=mean(data.meanErrMatForAllSub(:,1:8),1);
        centErrCI_ann=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        data=load(fullfile(dir,fileNameANN_RAD{i}));
        sizeErr_ann=mean(data.meanErrMatForAllSub(:,1:8),1);
        sizeErrCI_ann=1.96*(1/sqrt(nUsers))*std(data.meanErrMatForAllSub(:,1:8),0,1);
        data=load(fullfile(dir,fileNameCider_CENT{i}));
        
        cent=data.meanErrMatForAllSub(:,1:8);
        centErr=mean(cent(:));
        centErrCI=1.96*(1/sqrt(nUsers))*std(cent(:));
        data=load(fullfile(dir,fileNameCider_RAD{i}));
        sizee=data.meanErrMatForAllSub(:,1:8);
        sizeErr=mean(sizee(:));
        sizeErrCI=1.96*(1/sqrt(nUsers))*std(sizee(:));
        
        SampCostPixels=100*power_data_cross_ideal.sampling_cost_pixels/112^2;
        SampCostPower=power_data_cross_ideal.sampling_cost_power;
        CompCost_cycles=power_data_cross_ideal.computation_cost_cycles;
        CompCost_power=power_data_cross_ideal.computation_cost_power;
        Speed=power_data_cross_ideal.framerate;
        NIRCost=power_data_cross_ideal.nir_cost;
        
        table_ANN = table(centErr_ann',centErrCI_ann',sizeErr_ann',sizeErrCI_ann',NIRCost_ann',SampCostPower_ann',CompCost_cycles_ann',CompCost_power_ann',Speed_ann',SampCostPixels_ann','VariableNames',{'centErr_ann','centErrCI_ann','sizeErr_ann','sizeErrCI_ann','NIRCost_ann','SampCostPower_ann','CompCost_cycles_ann','CompCost_power_ann','Speed_ann','SampCostPixels_ann'});
        table_Cider = table(centErr',centErrCI',sizeErr',sizeErrCI',NIRCost',SampCostPower',CompCost_cycles',CompCost_power',Speed,'VariableNames',{'centErr','centErrCI','sizeErr','sizeErrCI','NIRCost','SampCostPower','CompCost_cycles','CompCost_power','Speed'});
        
    end
    
    if saveTableOn
        
        writetable(table_ANN,fullfile(dir,'table',strcat(fileNameList{i},'table_ANN.csv')));
        writetable(table_Cider,fullfile(dir,'table',strcat(fileNameList{i},'table_Cider.csv')));
    end
    
    %% Graph- pareto subplots for different metrics
%     metric_Cell_ann={NIRCost_ann',SampCostPower_ann',CompCost_power_ann',Speed_ann',SampCostPixels_ann'};
%     metric_Cell_cider={NIRCost',SampCostPower',CompCost_power',Speed',SampCostPixels'};
%     
    metric_Cell_ann={NIRCost_ann'+SampCostPower_ann'+CompCost_power_ann'};
    metric_Cell_cider={NIRCost'+SampCostPower'+CompCost_power'};
    
    ErrANN_Cell={centErr_ann',sizeErr_ann'};
    ErrCIDER_Cell={centErr',sizeErr'};
    
    if strcmp(fileNameList{i},'stage_')
        ErrIdealCross_Cell=ErrCIDER_Cell;
        metricIdealCross_Cell=metric_Cell_cider;
    end
end

if plotOn    
    
    if savePlotOn
        figure;
    end
    
    for m=1%:length(metricList)
        for k=1 %only center
            
            %subplot(sub1,sub2,2*(m-1)+k);
            plot(ErrANN_Cell{k},metric_Cell_ann{m});
            hold on;
            
            plot(ErrCIDER_Cell{k},metric_Cell_cider{m});
            plot(ErrIdealCross_Cell{k},metricIdealCross_Cell{m},'xb');
            
            xlabel(accuracyList{k});
            ylabel(metricList{m});
            h=title(sprintf('%s and %s', accuracyList{k}, metricList{m}));
            
            P = get(h,'Position');
            set(h,'Position',[P(1) P(2)+0.0001 P(3)]);
        end
    end
    hL=legend({'ANN','CIDER','Ideal Line'});
    newPosition = [0.95 0.95 0 0];
    newUnits = 'normalized';
    set(hL,'Position', newPosition,'Units', newUnits);
    
    suptitle('Stage Comparison of different metrics for ANN vs. CIDER');
    
    if savePlotOn
        plotFileName=strcat(fileNameList{i},'metricsGraphs_100Train');
    else
        plotFileName='';
    end
    
    plotMode='';
    plotFormatQ_final(dir,plotFileName,savePlotOn,plotMode);
    
    
end


%%

%plotFormatQ_final(fullfile(pathToSave,'../..'),plotFileName,saveOn);
