function [c,r]=chordal_search(X,xy_target,chord_length,thresh)
    c = [0, 0];
    r = 0;

    % Calculate row-column (rc) coordinates from xy
    rc_target = [xy_target(2), xy_target(1)];
    rc_target = round(rc_target);
    
    half_chord = ceil((chord_length - 1) / 2);
    
    v_ends = [max([-half_chord + rc_target(1) + 1, 1]), min([half_chord + rc_target(1) + 1, 111])];
    h_ends = [max([-half_chord + rc_target(2) + 1, 1]), min([half_chord + rc_target(2) + 1, 112])];

%     v_grad = diff(X( v_ends(1):v_ends(2), rc_target(2) ))';
%     h_grad = diff(X( rc_target(1), h_ends(1):h_ends(2) ));

    conv_op = [-1/2 0 1/2];
%     conv_op = [-1/12 -3/12 0 3/12 1/12];
    
    v_grad = conv(conv_op, X( v_ends(1):v_ends(2), rc_target(2) )');
    v_grad = v_grad(length(conv_op):end-length(conv_op)+1);
    h_grad = conv(conv_op, X( rc_target(1), h_ends(1):h_ends(2) ));
    h_grad = h_grad(length(conv_op):end-length(conv_op)+1);

    x_style = 'r-';
    y_style = 'r-';
    
    if (any(h_grad > 2))
        h_grad(abs(h_grad) > 2) = 0;
        x_style = 'm-';
    end
    
    if (any(v_grad > 2))
        v_grad(abs(v_grad) > 2) = 0;
        y_style = 'm-';
    end
    
    xy_target = round(xy_target);

    y_ends = [max([-half_chord + xy_target(2) + 1, 1]), min([half_chord + xy_target(2) + 1, 112])];
    x_ends = [max([-half_chord + xy_target(1) + 1, 1]), min([half_chord + xy_target(1) + 1, 111])];
    
    r_ratio = zeros(1,2);
    
    for i=1:2
        chords = zeros(4,2);

    %     all_grad = {h_grad, v_grad};
    %     
        % Find pupil chords from gradient lines
    %     for i=1:2
    %         grad = all_grad{i};
    %         [~, sort_idx] = sort(grad(:), 'descend');
    %         
    %         a = 1; 
    %         b = length(sort_idx);
    %         while (abs(sort_idx(a) - sort_idx(b)) < 5) && a < b
    %             a = a + 1;
    %             b = b - 1;
    %         end        
    %                 
    % %         max_pos = grad(sort_idx(a));
    % %         max_neg = grad(sort_idx(b));
    % %         abs_ratio = abs(max_pos / max_neg);
    %         
    %         % Gradient maxima need to be of opposite sign and close to the same
    %         % magnitude
    % %         if (max_pos * max_neg) > 0 || abs_ratio < 0.7 || abs_ratio > (10/7)
    % %             return
    % %         end
    % 
    %         if i == 1
    %             offset = floor(x_ends(1)) + floor(length(conv_op) / 2);
    %             chords(1,:) = [offset + sort_idx(a), xy_target(2)];
    %             chords(2,:) = [offset + sort_idx(b), xy_target(2)];
    %             plot(chords(1:2,1), chords(1:2,2), 'g-');
    %         else
    %             offset = floor(y_ends(1)) + floor(length(conv_op) / 2);
    %             chords(3,:) = [xy_target(1), sort_idx(a) + offset];
    %             chords(4,:) = [xy_target(1), sort_idx(b) + offset];
    %             plot(chords(3:4,1), chords(3:4,2), 'g-');
    %         end
    %     end

        chords(1:2,2) = round(xy_target(2));
        chords(3:4,1) = round(xy_target(1));

        y_vec = X( v_ends(1):v_ends(2), rc_target(2) )';
        x_vec = X( rc_target(1), h_ends(1):h_ends(2) );
        x_start = rc_target(2) - h_ends(1);
        y_start = rc_target(1) - v_ends(1);

        if (i==1)
            find_nzpair = @(x) find(x & [x(2:end), 0],1);

            chords(1,1) = round(xy_target(1)) + find_nzpair(x_vec(x_start+1:end) >= thresh) - 1;
            chords(2,1) = round(xy_target(1)) - find_nzpair(fliplr(x_vec(1:x_start-1)) >= thresh) - 1;
            chords(3,2) = round(xy_target(2)) + find_nzpair(y_vec(y_start+1:end) >= thresh) - 1;
            chords(4,2) = round(xy_target(2)) - find_nzpair(fliplr(y_vec(1:y_start-1)) >= thresh) - 1;
        else
            chords(1,1) = round(xy_target(1)) + find(x_vec(x_start+1:end) >= thresh,1) - 1;
            chords(2,1) = round(xy_target(1)) - find(fliplr(x_vec(1:x_start-1)) >= thresh,1) - 1;
            chords(3,2) = round(xy_target(2)) + find(y_vec(y_start+1:end) >= thresh,1) - 1;
            chords(4,2) = round(xy_target(2)) - find(fliplr(y_vec(1:y_start-1)) >= thresh,1) - 1;
        end

        % Make sure chords overlap
        if ( all(chords(1:2,1) < xy_target(1)) || all(chords(1:2,1) > xy_target(1)) || all(chords(3:4,2) > xy_target(2)) || all(chords(3:4,2) < xy_target(2)) )
            continue;
        end

        % Calculate chord midpoints to get pupil center
        x_mid = mean([chords(1,1), chords(2,1)]);
        y_mid = mean([chords(3,2), chords(4,2)]);

        c = [x_mid, y_mid];

        % Calculate pupil radius from each chord separately
        r1 = sqrt((x_mid - chords(1,1)) ^ 2 + (y_mid - chords(1,2)) ^ 2);
        r2 = sqrt((y_mid - chords(3,2)) ^ 2 + (x_mid - chords(3,1)) ^ 2);
        
        if (abs(chords(1,1) - chords(2,1)) < 4 || abs(chords(3,2) - chords(4,2)) < 4)
            continue;
        end

        % If the computed radii aren't very similar (i.e. the shape is not 
        % close to circular), we probably didn't actually find the pupil
        r_ratio(i) = abs(r1 / r2);
        
        if abs(r1 / r2) < 0.6 || abs(r1 / r2) > 1/0.6
            continue;
        end

        r = mean([r1, r2]);

        [circX, circY] = gen_circle(x_mid, y_mid, r);
        plot(circX, circY, 'b-','LineWidth',3);
        plot(x_mid, y_mid, 'b+', 'MarkerSize', 25,'LineWidth',3);
        
        return;
    end

    plot(x_ends, [xy_target(2), xy_target(2)], x_style);
    plot([xy_target(1), xy_target(1)], y_ends, y_style);
    plot(chords(1:2,1), chords(1:2,2), 'g-');
    plot(chords(3:4,1), chords(3:4,2), 'g-');
    
%     chords
%     thresh
%     r_ratio
%     input('Press enter to continue','s');
    return;
end