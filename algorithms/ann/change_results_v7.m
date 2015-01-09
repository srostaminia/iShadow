subjects = cellstr(ls);
origin = pwd;
data_names = {'c', 'ind', 'l', 'lambda', 'r', 'these_params', 'these_results'};

for i=1:length(subjects)
	subject = subjects{i};
	cd(sprintf('%s/full/results/', subject));

	sub_dirs = cellstr(ls);

	for j = 1:length(sub_dirs)
		sub_dir = sub_dirs{j};
		cd(sub_dir);

		reps = cellstr(ls);
		for j=1:length(reps)
			rep = reps{j};
			disp(sprintf('%s - %s - %s', subject, sub_dir, rep));
			clear(data_names{:});
			load(rep);
			save('-v7', sprintf('v7_%s', rep), data_names{:});
			clear(data_names{:});
		end

		cd('..');
	end

	cd(origin);
end
