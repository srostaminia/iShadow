function [pred,ann_used]=hybrid_nnline(X,ann_file,chord_length,data_name,thresh)
    load(ann_file,'ind','these_results');
    
    % Contrast adjustment
    X_adjust = X(:,:);
    X_adjust(:,ind(1:end-1)) = mean_contrast_adjust_nosave(X(:,ind(1:end-1)));
    
    scale_params = zeros(size(X,1),2);
    
%     X_display = mean_contrast_adjust_nosave(X);
    scale_params(:,1) = mean(X,2);
    X_display = bsxfun(@minus,X,mean(X,2));
    scale_params(:,2) = std(X_display,0,2);
    X_display = bsxfun(@rdivide,X_display,std(X_display,0,2));

    writerObj = VideoWriter(sprintf('%s_vid.mp4',data_name), 'MPEG-4');
    writerObj.FrameRate = 12;

    open(writerObj);

    pred = zeros(size(X,1),2);
    radii = zeros(size(X,1),1);
    ann_used = zeros(size(X,1),1);
    
    no_pupil = 1;
    for i = 1:size(X_adjust,1)
        hold off;
        imagesc(reshape(X_display(i,:), [111 112]));
        colormap('gray');
        axis equal;
        hold on;
        drawnow;
        
        % Run neural network (a) to get initial estimate or
        % (b) if line search failed to find pupil accurately
        if (i == 1) || no_pupil
            pred(i,:) = [112,111] .* logisticmlp_prediction(these_results.W, [X_adjust(i,:) 1], 7, 2);
%             pred(i,:) = ginput;
            ann_used(i) = 1;
            no_pupil = 0;
            
            [maskX, maskY] = ind2sub([111 112], ind);
            scatter(maskX, maskY, 'rs');
            plot(pred(i,1), pred(i,2),'b+','MarkerSize',25,'LineWidth',3);
            pred(i,:)
        else
            % Apply contrast adjustment parameters to the pixel threshold
            thresh_adjust = (thresh - scale_params(i,1)) / scale_params(i,2);
            [center, radius] = chordal_search(reshape(X_display(i,:), [111 112]), pred(i-1,:), chord_length,thresh_adjust);
            
            if (radius == 0)
                no_pupil = 1;
            else
                radii(i) = radius;
                pred(i,:) = center;
            end
        end
        
        title(sprintf('%d / %d',i,size(X_adjust,1)));
        
        drawnow;
        writeVideo(writerObj,getframe);
        
%         input('Press enter to continue','s');
        
        if (mod(i,500) == 0)
            disp(i)
        end
    end
    
    close;
    close(writerObj);
    
    save('-V7',sprintf('%s_results.mat',data_name),'chord_length','pred','radii','ann_used');
    
end