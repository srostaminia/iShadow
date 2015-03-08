function [total, led, cam, mcu] = calc_system_power(data, thresh)

conv_op = [ones([1,100]) * -1, 0, ones([1,100])];

conv_offset = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];

edge_detect = abs(conv(data, conv_op, 'valid'));

[~, edges] = findpeaks(edge_detect,'MINPEAKHEIGHT',thresh,'MINPEAKDISTANCE',100);

edges = edges + conv_offset(1);

gap_search = diff(edges);

first_gap = find(gap_search > 3000);

edges = edges(first_gap+1:end);

if mod(length(edges),2) == 0
    edges = edges(1:end-2);
end

power_ranges = edges(1:2:length(edges))';

power_regions = cell([1, length(power_ranges)-1]);
db_means = [];
for i=1:length(power_regions)
    if (mod(i,4) ~= 0)
        power_regions{i} = data(power_ranges(i):power_ranges(i+1))';
        db_means = [db_means; mean(power_regions{i})];
    else
        gap_size = edges((i * 2) + 1) - edges(i * 2);
        region_stop = floor(power_ranges(i+1) - (gap_size / 2));
        
        power_regions{i} = data(power_ranges(i):region_stop)';
        db_means = [db_means; mean(power_regions{i})];
    end
end

total = mean([power_regions{1:4:end}]);
mcu = mean([power_regions{4:4:end}]);

sans_cam = mean([power_regions{2:4:end}]);
sans_led = mean([power_regions{3:4:end}]);

led = total - sans_led;
cam = total - sans_cam;