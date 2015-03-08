function [power_data] = load_power(ind_size)

power_data = load(sprintf('~/Desktop/power_data/ind%d.csv',ind_size));
power_data = power_data(:,2);
power_data = power_data(1:300000);

conv_op = [ones([1,100]) * -1, 0, ones([1,100])];
conv_offset = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];
edge_detect = abs(conv(power_data(1:30000), conv_op, 'valid'));

figure;
plot(edge_detect);