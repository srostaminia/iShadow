

% -----------------START OF CONFIGURATION SETTINGS---------------------------------------------------------

% Configure these two values for the experiment
experiment_descriptor = {'irb_pupil_percent_0-2_98'};

experiment_descriptor = strcat('_',  experiment_descriptor);
experiment_title_list = strcat('awesomeness' , experiment_descriptor);
modelNameList={'cider','ann'};
subjects={'addison','duncan'};%['duncan','kaituo','malai','mike'] %sean
obj='pupil';
lightings={'_calib_','_dark_'};


names=[];

for i=1:length(lightings)
    names=[names strcat(subjects,lightings{i},obj)];
end

names(strcmp(names,'addison_dark_pupil'))=[];

%names=['addison_calib_pupil']+names

uniList = {'uniquefy_0', 'uniquefy_1'};

%250 for gaze

lambdas ={'0.000100','0.000215','0.000464','0.001000','0.002154','0.004642','0.010000','0.021544','0.046416','0.100000'};
% lambdas = [0.001000, 0.010000, 0.100000]

% -----------------END OF CONFIGURATION SETTINGS-----------------------------------------------------------
work_dir = pwd;

%work/awesomeness_irb_??
path_sh=work_dir;

for exp=1:length(experiment_title_list)
    
    experiment_title=experiment_title_list{exp};
    path_sh=fullfile(work_dir, experiment_title);
    
    cd (experiment_title);
    
    for n=1:length(names)
        name=names{n};
        cd (name);%awesomeness_irb_??/addison_calib_pupil
        
        for u=1:length(uniList)
            uni=uniList{u};
            cd (fullfile(uni,'results'));%uniquefy_0/results
            
            for lam=1:length(lambdas)
                lamFile=strcat('subset_l1_init_strips_k7_lambda',lambdas{lam});
                cd (lamFile);
                
                ori=pwd; %currently, in rep file directory

                for i=1:5
                    fileToSave=sprintf('v7_rep%i.mat',i);
 

                    %save v7 file
                    if exist(fileToSave, 'file') == 2
                        clear(data_names{:});
                        load(sprintf('rep%i.mat',i));
                        save('-v7', fileToSave, data_names{:});
                        clear(data_names{:});
                        
                        disp({'ERROR: ',modelName,' already ran before.'});
                        exit;
                    else
                        disp(sprintf('%s - %s - %s - %s', name, uniList{u}, lambdas{lam}, strcat('rep',i)));
                    end
                    
                    %run model
                    cd(path_sh);
                    mkdir('parallelFiles');
                    cd('parallelFiles');
                    
                    for ind=1:length(modelNameList)
                        modelName= modelNameList{modelInd};
                        data_title = strcat(experiment_descriptor,'_',name,'_',uniList{u}, '_',lam,'_',modelName,'_rep' , str(i));
                        
                        mkdir(data_title);
                        cd(data_title);
                        
                        modelFileName=sprintf('%s_rep%i',modelName,i);
                        
                        if exist(modelFileName, 'file') ~= 2
                            fileID = fopen(fullfile(path_sh , data_title ,'.sh'),'w');
                            fprintf(fileID,'#!/bin/bash\n');
                            fprintf(fileID,sprintf('octave %s/run_model/run_%s.m\n',work_dir, modelFileName));
                            fclose(fileID);
                            
                            command=['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title,'.sh'];
                            [status,cmdout] = system(command);
                        end
                        cd('..');
                    end
                end
                cd(ori) %in lambda file
            end
            cd('..')%in one unique file
        end
        cd('../..')%in subject file
    end
    cd('..')%in irb file
end
cd(work_dir);

