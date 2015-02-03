%function [pred,radii]=ann(X,ann_file,nDim,scaleVect)
function [pred]=ann(X,ann_file,nDim,scaleVect)

%pred= predicted pupil center
    
    %%%these 4 lines copied from cider.m 
    pred=zeros(size(X,1),2);
    radii = zeros(size(X,1),1);
    
    load(ann_file,'ind','these_results');
    % Contrast adjustment
    X_adjust = X(:,:);
    X_adjust(:,ind(1:end-1)) = mean_contrast_adjust_nosave(X(:,ind(1:end-1)));

    %%%
    for i = 1:size(X,1)
        predVect = scaleVect .* logisticmlp_prediction(these_results.W, [X_adjust(i,:) 1], 7, nDim);
        pred(i,:) = predVect(1:2);
            
            if  size(predVect)==3
                radii(i) = predVect(3);
            end
    end

    if isequal(size(pred),[size(X,1) 2])
        fprintf('pred dimension correct in ann.m\n\n');
    else
        fprintf('ERROR: pred dimension in ann.m\n\n');
    end
end