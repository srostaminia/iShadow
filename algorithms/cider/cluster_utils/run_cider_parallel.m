subjects = cellstr(ls);
origin = pwd;

for i=1:length(subjects)
	subject = subjects{i};

	data_file = strcat('~/work/training_sets/',subject,'.mat');
	if ~exist(data_file,'file')
		fprintf('ERROR: Data file %s not found', data_file);
		return;
	end

	cd(subject);
	if exist('cider_run','dir')
		fprintf('Cider run data found in %s, skipping',subject);
		continue;
	end

	mkdir('cider_run');
	cd('cider_run');

	symlink('/mnt/nfs/work1/marlin/amayberr/cider_thread.m','cider_thread.m');

	script_name = strcat('run_cider_',subject,'.sh');

	ofid = fopen(script_name,'w');
	fputs(ofid, '#!/bin/bash\n');
	fputs(ofid, sprintf('octave --eval "cider_thread(%s)"',data_file));
	fclose(ofid);

	system(sprintf('qsub -cwd -o stdout.txt -e stderr.txt %s', script_name));

	cd(origin);
end