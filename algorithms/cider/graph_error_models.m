function graph_error_models(subList, rootDir)

%rootDir='~/iShadow/algorithms/cider/data/awesomeness_irb_pupil'
%graph uniquefy0 and uniquefy1 separately->unique
%higher lambda->more regularization/restriction->smaller subsampling 
 
uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'}

errList={};

nRep=5;

for i=1:length(subList)
    
    subName=subList{i}; %subject name
       
    for j=1:length(uniFolderList)
        
        uni=uniFolderList{j};
        
        for k=1:length(lambdaFolderList)
            
            lambdaStr=lambdaFolderList{k};
        
            for m=1:nRep
            
                dir= fullfile(rootDir,strcat(subName,'_calib_pupil'),uni,'results',lambdaStr,strcat('cider_','rep',num2str(m)));    
                
                data=load(dir);
                err=[errList,data.err];
                
                
            end
        end
    end
end
                

