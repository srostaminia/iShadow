clear;
close all;

dir='/Users/ytun/Google Drive/IMPORTANT_VISION/Data_Err';
addpath('/Users/ytun/iShadow/evaluation/Error_graphs');

nUsers=5;%size(centErr_ann,1);

%% TABLE

lightingList={'dark','calib'};%,'ohsocold'};
stage_list={'ideal','switching'};%,'IdealLine'};'aggregate',
objToPlotList={'pupilcentPix','radius'}; %pupilLoc_deg,pupilLoc_pixel,area,radius
modelNameList={'ann','cider'};

%powerD.power_data_ann.sampling_cost_pixels
%nUsers=error_data.nUser;%size(centErr_ann,1);

% strings
%fileNameList={'stage_','switching_'};
accuracyList={'Pupil Center', 'Pupil Size'};
metricList={'NIR Cost','Sampling Power Cost','Computation Power Cost','Speed','Sampling Cost (pixels)'};

sub1=2; sub2=5;

savePlotOn=1;
saveTableOn=1;
plotOn=1;

tableName='finalTable';
ErrIdealCross_Cell={};

power_err_data=struct;

if saveTableOn
    mkdir(fullfile(dir,'table'));
    mkdir(fullfile(dir,'finalTable'));
end

for m=1:length(lightingList)
    
    lighting=lightingList{m};
    
    freqD=load(fullfile(dir,'frequency.mat'));
    powerD=load(fullfile(dir,'table_power.mat'));
    errorD=load(fullfile(dir,strcat(lighting,'_error_pplot.mat')));
    
    for i=1:length(stage_list)
        %ROWS
        %CentErrCI=
        %sizeErrCI=
        
        SampCostPixels_ann=100*powerD.power_data_ann.sampling_cost_pixels(:,1:8);%/112^2;
        SampCostPower_ann=powerD.power_data_ann.sampling_cost_power;
        CompCost_cycles_ann=powerD.power_data_ann.computation_cost_cycles;
        CompCost_power_ann=powerD.power_data_ann.computation_cost_power;
        Speed_ann=powerD.power_data_ann.framerate;
        NIRCost_ann=powerD.power_data_ann.nir_cost;
        
        disp(i);
        
        centErr_ann=errorD.errData.(stage_list{i}).ann.pupilcentPix.err(:,1:8);
        centErrCI_ann=errorD.errData.(stage_list{i}).ann.pupilcentPix.std(:,1:8);
        
        sizeErr_ann=errorD.errData.(stage_list{i}).ann.radius.err(:,1:8);
        sizeErrCI_ann=errorD.errData.(stage_list{i}).ann.radius.std(:,1:8);
        
        pixels_ann=errorD.errData.(stage_list{i}).ann.indper(:,1:8);
        
        centErr_cider=errorD.errData.(stage_list{i}).cider.pupilcentPix.err(:,1:8);
        centErrCI_cider=errorD.errData.(stage_list{i}).cider.pupilcentPix.std(:,1:8);
        
        sizeErr_cider=errorD.errData.(stage_list{i}).cider.radius.err(:,1:8);
        sizeErrCI_cider=errorD.errData.(stage_list{i}).cider.radius.std(:,1:8);
        
        %freq_cider=errorD.errData.(stage_list{i}).ann.indper(:,1:8);
        
        SampCostPixels_cider=100*powerD.power_data_cider.sampling_cost_pixels;%/112^2;
        SampCostPower_cider=powerD.power_data_cider.sampling_cost_power;
        CompCost_cycles_cider=powerD.power_data_cider.computation_cost_cycles;
        CompCost_power_cider=powerD.power_data_cider.computation_cost_power;
        Speed_cider=powerD.power_data_cider.framerate;
        NIRCost_cider=powerD.power_data_cider.nir_cost;
        
        ANNUsedPerc=mean(freqD.FreqAnnUsedMatForAllSub(:,1:8),1);
        CiderUsedPerc=mean(freqD.FreqLineUsedMatForAllSub(:,1:8),1);
        
        
        
        if strcmp(stage_list{i},'ideal')
            
            centErr_cider=mean(errorD.errData.(stage_list{i}).cider.pupilcentPix.err(:,1:8));
            sizeErr_cider=mean(errorD.errData.(stage_list{i}).cider.radius.err(:,1:8));
            
            SampCostPixels_cider=100*powerD.power_data_cross_ideal.sampling_cost_pixels;%/112^2;
            SampCostPower_cider=powerD.power_data_cross_ideal.sampling_cost_power;
            CompCost_cycles_cider=powerD.power_data_cross_ideal.computation_cost_cycles;
            CompCost_power_cider=powerD.power_data_cross_ideal.computation_cost_power;
            Speed_cider=powerD.power_data_cross_ideal.framerate;
            NIRCost_cider=powerD.power_data_cross_ideal.nir_cost;
            
            
        end
        
        power_ann=NIRCost_ann+SampCostPower_ann+CompCost_power_ann;
        power_cider=NIRCost_cider+SampCostPower_cider+CompCost_power_cider;
        
                table_ANN = table(power_ann',centErr_ann',centErrCI_ann',sizeErr_ann',sizeErrCI_ann',NIRCost_ann',SampCostPower_ann',CompCost_cycles_ann',CompCost_power_ann',Speed_ann',SampCostPixels_ann',...
                    'VariableNames',{'power_ann','centErr_ann','centErrCI_ann','sizeErr_ann','sizeErrCI_ann','NIRCost_ann','SampCostPower_ann','CompCost_cycles_ann','CompCost_power_ann','Speed_ann','SampCostPixels_ann'});
%         table_ANN = table(centErr_ann',power_ann',centErrCI_ann',...
%             'VariableNames',{'centErr_ann','power_ann','centErrCI_ann'});
%         
%         if strcmp(stage_list{i},'switching')
%             table_Cider = table(centErr_cider',power_cider',centErrCI_cider',...
%                 'VariableNames',{'centErr_cider','power_cider','centErrCI_cider'});
%         elseif strcmp(stage_list{i},'ideal')
%             table_Cider = table(centErr_cider',power_cider',...
%                 'VariableNames',{'centErr_cider','power_cider'});
%         end
        if strcmp(stage_list{i},'switching')
        
           table_Cider = table(power_cider',ANNUsedPerc',CiderUsedPerc',centErr_cider',centErrCI_cider',sizeErr_cider',sizeErrCI_cider',NIRCost_cider',SampCostPower_cider',CompCost_cycles_cider',CompCost_power_cider',Speed_cider',SampCostPixels_cider',...
                'VariableNames',{'power_cider','ANNUsedPerc','CiderUsedPerc','centErr_cider','centErrCI_cider','sizeErr_cider','sizeErrCI_cider','NIRCost_cider','SampCostPower_cider','CompCost_cycles_cider','CompCost_power_cider','Speed_cider','SampCostPixels_cider'});
       
        elseif strcmp(stage_list{i},'ideal')
            table_Cider = table(power_cider',centErr_cider',sizeErr_cider',NIRCost_cider',SampCostPower_cider',CompCost_cycles_cider',CompCost_power_cider',Speed_cider',SampCostPixels_cider',...
                'VariableNames',{'power_cider','centErr_cider','sizeErr_cider','NIRCost_cider','SampCostPower_cider','CompCost_cycles_cider','CompCost_power_cider','Speed_cider','SampCostPixels_cider'});
        end
        
        if saveTableOn
            writetable(table_ANN,fullfile(dir,tableName,strcat(stage_list{i},'_',lighting,'_table_ANN.csv')));
            writetable(table_Cider,fullfile(dir,tableName,strcat(stage_list{i},'_',lighting,'_table_Cider.csv')));
        end
        
        
        %% Graph- pareto subplots for different metrics
        
       
    end
end

% if plotOn
% 
%     figure;
%     plot(ErrANN_Cell{k},metric_Cell_ann{m});
%     hold on;
%     
%     if strcmp(stage_list{i},'ideal')
%         plot(ErrIdealCross_Cell{k},metricIdealCross_Cell{m},'xb');
%     elseif strcmp(stage_list{i},'switching')
%         plot(ErrCIDER_Cell{k},metric_Cell_cider{m});
%     end
%     xlabel(accuracyList{k});
%     ylabel(metricList{m});
%     h=title(sprintf('%s and %s', accuracyList{k}, metricList{m}));
%     
%     P = get(h,'Position');
%     set(h,'Position',[P(1) P(2)+0.0001 P(3)]);
% 
% 
%     hL=legend({'ANN','CIDER','Ideal'});
%     newPosition = [0.95 0.95 0 0];
%     newUnits = 'normalized';
%     set(hL,'Position', newPosition,'Units', newUnits);
% 
%     suptitle('Stage Comparison of different metrics for ANN vs. CIDER');
% 
%     if savePlotOn
%         plotFileName=strcat(stage_list{i},'_',lighting,'_metricsGraphs');
%     else
%         plotFileName='';
%     end
% 
%     plotMode='subplot';
%     plotFormatQ_final(dir,plotFileName,savePlotOn,plotMode);
% 
% end






%%

%plotFormatQ_final(fullfile(pathToSave,'../..'),plotFileName,saveOn);
