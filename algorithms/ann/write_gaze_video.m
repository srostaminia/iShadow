function save_model(X,Out,Wfull,params,dirs)

res = [112,112];

writerObj = VideoWriter(sprintf('videos/gaze_video%d.avi',f));
set(writerObj,'FrameRate',4, 'Quality', 90);
open(writerObj);

  figure(1);colormap(gray);
  set(gcf,'position',[0 0 112*4 112*2]);
  set(gcf,'Renderer','zbuffer');
  set(gca,'nextplot','replacechildren');

  for l=1:length(eye_list);

    tic
    eye_img   = double(imread(sprintf('%s%s',eye_dir,eye_list(l).name)));
    out_img   = imread(sprintf('%s%s',out_dir,out_list(l).name));

    if(f>1)
      eye_img = imresize(eye_img(1:end-40,4:end-1),[112,112]);
    end
        
    pred =  logisticmlp_prediction(W10,[eye_img(:);1]'/255,[0 0],7);
    hold off;
    imagesc([fliplr(eye_img), out_img]);
    hold on;
    plot(112+112*pred(1),112*pred(2),'r+','markersize',20,'linewidth',3);
    axis off;
    drawnow;

    frame = getframe;
    writeVideo(writerObj,frame);

    t=toc;
    %pause(max(0,1/12-t));
  end

  close(writerObj);
end