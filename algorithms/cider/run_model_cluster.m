function run_model_cluster(repNum,modelName,rep_dir, input_dir,do_uniquefy,nDim,scaleVect,sub_model,contrast_method,percentile,testIndOn,thirdVarName)

    data=load(input_dir);

    X=data.X;
    gout=data.gout;
    sqrtRadEllipse=data.sqrtRadEllipse;
    trueArea=data.area;

    %test_ind -> cell(1,5)

    if testIndOn
        
        [~, test_ind] = gen_crossval_ind(X, do_uniquefy, gout);
        
        XCell=cell(1,5);
        goutCell=cell(1,5);
        sqrtRadEllipseCell=cell(1,5);
        
        XCell{repNum}=X(test_ind{repNum},:);
        goutCell{repNum}=gout(test_ind{repNum},:);
        
        if nDim==3
            sqrtRadEllipseCell{repNum}=sqrtRadEllipse(test_ind{repNum},:);
            trueAreaCell{repNum}=trueArea(test_ind{repNum},:);
        end
        

    end
    %fprintf('\n');

    %fprintf('%s - %d\n',lambda_folders{i},repNum);

    if testIndOn
        X=XCell{repNum};
    end

    rep_file=fullfile(rep_dir,sprintf('v7_rep%d.mat',sub_model,repNum));

    if strcmp(modelName,'cider')
        [ind,chord_length,center.pred,thirdVar,ann_used] = cider(X, rep_file, 400, 0.22, 'circle_edge', 0,nDim,scaleVect,contrast_method,percentile);

    elseif strcmp(modelName,'ann')
        [ind,center.pred,thirdVar]= ann(X, rep_files{repNum},nDim,scaleVect,contrast_method,percentile);
    end

    if testIndOn
        gout=goutCell{repNum};

        if nDim==3
            sqrtRadEllipse=sqrtRadEllipseCell{repNum};
            trueArea=trueAreaCell{repNum};
        end
    end

    center.true=gout;
    radii.true=sqrtRadEllipse;
    area.true=trueArea;

    % % %
    %Cider model
    filter = logical(sum(center.pred,2));
    %CENTER
    center.diff.raw = sqrt(sum((center.true - center.pred).^2,2));

    mean_model.center.diff = mean(center.diff.raw(filter,:));
    mean_model.center.diff_deg=0.32*mean_model.center.diff;

    %%subsampled pixels
    indActivePercent=100*length(ind)/(112*112);



    % % %
    %line model

    if strcmp(modelName,'cider')
        totalFrame=size(ann_used,1);
        perc_AnnUsed=100*sum(ann_used)/totalFrame;


        filter_Line =  filter & ~ann_used;
        %CENTER
        mean_line.center.diff = mean(center.diff.raw(filter_Line,:));
        mean_line.center.diff_deg=0.32*mean_line.center.diff;


        %line model
        filter_ann = filter &  ann_used;
        %CENTER
        mean_ann.center.diff = mean(center.diff.raw(filter_ann,:)); %pixel
        mean_ann.center.diff_deg=0.32*mean_ann.center.diff; %degree

        %subsampled pixels
        perc_LineUsed=100-perc_AnnUsed;

    end



    if nDim==3
        if strcmp(thirdVarName,'radius')
            radii.pred=thirdVar;
            area.pred=pi*radii.pred.^2;
        elseif strcmp(thirdVarName,'area')
            area.pred=thirdVar;
        end


        %AREA

        area.diff=area.pred-area.true; %in pixels
        area.diff_perc=100*abs(area.diff)./(area.true);

        %radius
        radius.diff=radii.true-radii.pred;

        mean_model.area.diff_perc=mean(area.diff_perc(filter,:));
        mean_model.radius.diff=mean(radius.diff(filter,:));

        if strcmp(modelName,'cider')
            mean_line.area.diff_perc=mean(area.diff_perc(filter_Line,:));
            mean_ann.area.diff_perc=mean(area.diff_perc(filter_ann,:));

            mean_line.radius.diff=mean(radius.diff(filter_Line,:));
            mean_ann.radius.diff=mean(radius.diff(filter_ann,:));

            save('-V7',fullfile(rep_dir,sprintf('cider_%srep%d.mat',sub_model,repNum)),...
                'ind','chord_length','center','radii','ann_used','filter','center','area','mean_model',...
                'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed','mean_ann','filter_ann');

        elseif strcmp(modelName,'ann')

            save('-V7',fullfile(rep_dir,sprintf('ann_%srep%d.mat',sub_model,repNum)),...
                'ind','center','radii','filter','center','area','mean_model',...
                'indActivePercent');

        end


    else
        if strcmp(modelName,'cider')

            save('-V7',fullfile(rep_dir,sprintf('cider_%srep%d.mat',sub_model,repNum)),...
                'ind','chord_length','center','ann_used','filter','mean_model',...
                'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed','mean_ann','filter_ann');

        elseif strcmp(modelName,'ann')
            save('-V7',fullfile(rep_dir,sprintf('ann_%srep%d.mat',sub_model,repNum)),...
                'ind','center','filter','mean_model',...
                'indActivePercent');

        end

    end
   
end