function [best_c,best_r]=circle_edge_model(X,xy_target,chord_length,thresh,last_r,do_plot)

    % Calculate row-column (rc) coordinates from xy
    rc_target = [xy_target(2), xy_target(1)];
    rc_target = round(rc_target);
    
    half_chord = ceil((chord_length - 1) / 2);
    
    v_ends = [max([-half_chord + rc_target(1) + 1, 1]), min([half_chord + rc_target(1) + 1, 111])];
    h_ends = [max([-half_chord + rc_target(2) + 1, 1]), min([half_chord + rc_target(2) + 1, 112])];
    
    xy_target = round(xy_target);

    y_ends = [max([-half_chord + xy_target(2) + 1, 1]), min([half_chord + xy_target(2) + 1, 112])];
    x_ends = [max([-half_chord + xy_target(1) + 1, 1]), min([half_chord + xy_target(1) + 1, 111])];
    
    v_line = X( v_ends(1):v_ends(2), rc_target(2) )';
    h_line = X( rc_target(1), h_ends(1):h_ends(2) );
    
    v_pupil_possible = find_pupil_edge(xy_target(2), v_line, thresh) + y_ends(1) - 1;
    h_pupil_possible = find_pupil_edge(xy_target(1), h_line, thresh) + x_ends(1) - 1;
    
    if do_plot == 1
        plot(x_ends, [xy_target(2), xy_target(2)], 'r-');
        plot([xy_target(1), xy_target(1)], y_ends, 'r-');
    end
    
    best_v = 0; best_h = 0; best_ratio = 0; best_r = 0; best_c = 0;
    
    for i=1:size(v_pupil_possible,1)
        v_pupil = v_pupil_possible(i,:);
        
        for j=1:size(h_pupil_possible,1)
            h_pupil = h_pupil_possible(j,:);

            chords = zeros(4,2);
            chords(1:2,1) = h_pupil;
            chords(1:2,2) = round(xy_target(2));
            chords(3:4,1) = round(xy_target(1));
            chords(3:4,2) = v_pupil;

            % Calculate chord midpoints to get pupil center
            x_mid = mean(h_pupil);
            y_mid = mean(v_pupil);

            c = [x_mid, y_mid];

            % Calculate pupil radius from each chord separately
            r1 = sqrt((x_mid - chords(1,1)) ^ 2 + (y_mid - chords(1,2)) ^ 2);
            r2 = sqrt((y_mid - chords(3,2)) ^ 2 + (x_mid - chords(3,1)) ^ 2);

            if (abs(chords(1,1) - chords(2,1)) < 4 || abs(chords(3,2) - chords(4,2)) < 4)
                continue;
            end

            ratio = r1 / r2;
            if ratio < 0.6 || ratio > 1/0.6 || abs(ratio - 1) > abs(best_ratio - 1)
                continue;
            end

            r = mean([r1, r2]);

            if last_r ~= 0 && (r / last_r < 0.75 || r / last_r > 1/0.75)
                continue;
            end
            
            best_ratio = ratio;
            best_v = i;
            best_h = j;
            best_r = r;
            best_c = c;
        end
    end
    
    if (best_h == 0)
        best_r = 0;
        if do_plot == 1
            plot(chords(1:2,1), chords(1:2,2), 'g-');
            plot(chords(3:4,1), chords(3:4,2), 'g-');
        end
        return;
    end
    
    if do_plot == 1
        h_pupil = h_pupil_possible(best_h,:);
        v_pupil = v_pupil_possible(best_v,:);

        chords = zeros(4,2);
        chords(1:2,1) = h_pupil;
        chords(1:2,2) = round(xy_target(2));
        chords(3:4,1) = round(xy_target(1));
        chords(3:4,2) = v_pupil;
        
        plot(chords(1:2,1), chords(1:2,2), 'g-');
        plot(chords(3:4,1), chords(3:4,2), 'g-');

        [circX, circY] = gen_circle(best_c(1), best_c(2), best_r);
        plot(circX, circY, 'b-','LineWidth',3);
        plot(best_c(1), best_c(2), 'b+', 'MarkerSize', 25,'LineWidth',3);
    end
    
    %     chords
    %     thresh
    %     r_ratio
    %     input('Press enter to continue','s');
    
    return;
end