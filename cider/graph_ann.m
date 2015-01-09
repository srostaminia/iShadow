function [pred,radii,ann_used]=graph_ann(X,ann_file,data_name,gout,make_vid)
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

    pred = zeros(size(X,1),3);
    radii = zeros(size(X,1),1);
    ann_used = zeros(size(X,1),1);
    
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
        
        pred(i,:) = [112,111,112] .* logisticmlp_prediction(these_results.W, [X_adjust(i,:) 1], 7, 3);
%             pred(i,:) = ginput;
        ann_used(i) = 1;

        if make_vid == 1
%             [maskX, maskY] = ind2sub([111 112], ind);
%             scatter(maskX, maskY, 'rs');
            plot(pred(i,1), pred(i,2),'b+','MarkerSize',25,'LineWidth',3);
            [circX, circY] = gen_circle(pred(i,1), pred(i,2), pred(i,3));
            plot(circX, circY, 'b-','LineWidth',3);
            
            plot(gout(i,1), gout(i,2),'g+','MarkerSize',25,'LineWidth',3);
            [circX, circY] = gen_circle(gout(i,1), gout(i,2), gout(i,3));
            plot(circX, circY, 'g-','LineWidth',3);
            
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