function [c,r]=ellipse_model(X,xy_target,chord_length,thresh)
    c = [0, 0];
    r = 0;

    % Calculate row-column (rc) coordinates from xy
    rc_target = [xy_target(2), xy_target(1)];
    rc_target = round(rc_target);
    
    half_chord = ceil((chord_length - 1) / 2);
    
    v_ends = [max([-half_chord + rc_target(1) + 1, 1]), min([half_chord + rc_target(1) + 1, 111])];
    h_ends = [max([-half_chord + rc_target(2) + 1, 1]), min([half_chord + rc_target(2) + 1, 112])];
    
    xy_target = round(xy_target);

    y_ends = [max([-half_chord + xy_target(2) + 1, 1]), min([half_chord + xy_target(2) + 1, 112])];
    x_ends = [max([-half_chord + xy_target(1) + 1, 1]), min([half_chord + xy_target(1) + 1, 111])];
    
    r_ratio = zeros(1,2);
    
    for i=1:2
        chords = zeros(4,2);

        chords(1:2,2) = round(xy_target(2));
        chords(3:4,1) = round(xy_target(1));

        y_vec = X( v_ends(1):v_ends(2), rc_target(2) )';
        x_vec = X( rc_target(1), h_ends(1):h_ends(2) );
        x_start = rc_target(2) - h_ends(1);
        y_start = rc_target(1) - v_ends(1);

        % First try using the first two pixels in a row that fall below the threshold   
        if (i==1)
            find_nzpair = @(x) find(x & [x(2:end), 0],1);

            chords(1,1) = round(xy_target(1)) + find_nzpair(x_vec(x_start+1:end) >= thresh) - 1;
            chords(2,1) = round(xy_target(1)) - find_nzpair(fliplr(x_vec(1:x_start-1)) >= thresh) - 1;
            chords(3,2) = round(xy_target(2)) + find_nzpair(y_vec(y_start+1:end) >= thresh) - 1;
            chords(4,2) = round(xy_target(2)) - find_nzpair(fliplr(y_vec(1:y_start-1)) >= thresh) - 1;
        else
            % If that doesn't give a good fit, try looking for the first single pixel that falls below the threshold
            chords(1,1) = round(xy_target(1)) + find(x_vec(x_start+1:end) >= thresh,1) - 1;
            chords(2,1) = round(xy_target(1)) - find(fliplr(x_vec(1:x_start-1)) >= thresh,1) - 1;
            chords(3,2) = round(xy_target(2)) + find(y_vec(y_start+1:end) >= thresh,1) - 1;
            chords(4,2) = round(xy_target(2)) - find(fliplr(y_vec(1:y_start-1)) >= thresh,1) - 1;
        end

        % Make sure chords overlap (probably redundant)
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