function [chord_length,pred,radii,ann_used]=cider(X,ann_file,chord_length,thresh,model,make_vid)
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

    if make_vid == 1
        writerObj = VideoWriter(sprintf('%s_vid.mp4',data_name), 'MPEG-4');
        writerObj.FrameRate = 12;

        open(writerObj);
        
        figure;
    end

    pred = zeros(size(X,1),2);
    radii = zeros(size(X,1),1);
    ann_used = zeros(size(X,1),1);
    
    no_pupil = 1;
    for i = 1:size(X_adjust,1)
        if make_vid == 1
            hold off;
            imagesc(reshape(X_display(i,:), [111 112]));
            colormap('gray');
            axis equal;
            axis off;
            hold on;
            title(sprintf('%d / %d',i,size(X_adjust,1)));
            drawnow;
        end
        
        % Run neural network (a) to get initial estimate or
        % (b) if line search failed to find pupil accurately
        if (i == 1) || no_pupil
            pred(i,:) = [112,111] .* logisticmlp_prediction(these_results.W, [X_adjust(i,:) 1], 7, 2);
%             pred(i,:) = ginput;
            ann_used(i) = 1;
            no_pupil = 0;
            
            if make_vid == 1
                [maskX, maskY] = ind2sub([111 112], ind);
                scatter(maskX, maskY, 'rs');
                plot(pred(i,1), pred(i,2),'b+','MarkerSize',25,'LineWidth',3);
            end
%             pred(i,:)
        else
            % Apply contrast adjustment parameters to the pixel threshold
            thresh_adjust = (thresh - scale_params(i,1)) / scale_params(i,2);
            
            switch model
                case 'circle_thresh'
                    [center, radius] = circle_thresh_model(reshape(X_display(i,:), [111 112]), pred(i-1,:), chord_length,thresh_adjust);

                    if (radius == 0)
                        no_pupil = 1;
                    else
                        radii(i) = radius;
                        pred(i,:) = center;
                    end
                case 'circle_edge'
                    [center, radius] = circle_edge_model(reshape(X_display(i,:), [111 112]), pred(i-1,:), chord_length, thresh, radii(i-1), make_vid);

                    if (radius == 0)
                        no_pupil = 1;
                    else
                        radii(i) = radius;
                        pred(i,:) = center;
                    end
                case 'ellipse'
                    [center, major, minor] = ellipse_model(reshape(X_display(i,:), [111 112]), pred(i-1,:), chord_length,thresh_adjust);
                    
                    if (major * minor == 0)
                        no_pupil = 1;
                    else
                        radii(i) = major;
                        pred(i,:) = center;
                    end
                case 'gaussian'
                    gaussian_model(reshape(X_display(i,:), [111 112]), pred(i-1,:), thresh_adjust);
                    
                    no_pupil = 1;
                otherwise
                    error('Invalid model "%s" specified\n"', model);
            end
        end
        
        if (make_vid == 1)
            drawnow;
            writeVideo(writerObj,getframe);
        end
        
%         i
%         pred(i,:)
%         input('Press enter to continue','s');
        
%         if (mod(i,500) == 0)
%             disp(i)
%         end
    end
    
    if (make_vid == 1)
        close;
        close(writerObj);
    end
    
end