function [X_total,Out_total,gout_total,hold_out_size] = join_eye_data(hold_out_idx)

names = {"addison2", "dan", "james", "john", "keith", "luis", "niri", "michelle", "steve", "seth"};

X_total=[];
Out_total=[];
gout_total=[];
total_size = 0;
hold_out_size = 0;

matfile = sprintf('~/training_sets/eye_data_%s_auto',names{hold_out_idx})
load(matfile);

gout = gout(1:size(X,1),:);
gout_total = [gout_total; gout];

X_total = [X_total; X];

Out_total = [Out_total; Out];

total_size += size(X,1);
hold_out_size = size(X,1);

for i = 1:size(names,2)
	if i == hold_out_idx
		continue
	end

	matfile = sprintf('~/training_sets/eye_data_%s_auto',names{i})
	load(matfile);

	gout = gout(1:size(X,1),:);
	gout_total = [gout_total; gout];

	X_total = [X_total; X];

	Out_total = [Out_total; Out];

	total_size += size(X,1);
end

total_size