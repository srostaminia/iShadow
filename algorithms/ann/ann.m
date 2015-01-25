function pred=ann(X,ann_file)

%pred= predicted pupil center
    
    %%%these 4 lines copied from cider.m 
    pred=zeros(size(X,1),2);
    
    load(ann_file,'ind','these_results');
    % Contrast adjustment
    X_adjust = X(:,:);
    X_adjust(:,ind(1:end-1)) = mean_contrast_adjust_nosave(X(:,ind(1:end-1)));

    %%%
    for i = 1:size(X,1)
        pred(i,:) = [112,111] .* logisticmlp_prediction(these_results.W, [X_adjust(i,:) 1], 7, 2);
    end

    if isequal(size(pred),[size(X,1) 2])
        fprintf('pred dimension correct in ann.m\n\n');
    else
        fprintf('ERROR: pred dimension in ann.m\n\n');
    end
end
