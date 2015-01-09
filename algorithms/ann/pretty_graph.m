function pretty_graph(hf,ha);
  %set(gca,'GridLineStyle','-');
  %set(gca,'XColor',[0.5 0.5 0.5]);
  %set(gca,'YColor',[0.5 0.5 0.5]);
  set(findall(hf,'type','text'),'fontSize',20,'fontname','times')
  set(findall(ha,'type','text'),'fontSize',20,'fontname','times')
  set(ha,'fontSize',16);