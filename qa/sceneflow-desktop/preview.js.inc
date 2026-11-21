// Visual SceneFlow desktop preview only; no camera stream, PII, storage or external calls.
(function(){
'use strict';
const root=document.getElementById('vo-scene'),picker=document.getElementById('vo-scene-template');
if(!root||!picker)return;
const ideas=[
['Gözlenebilen olay sayısı, kamera çalışma süresiyle birlikte değerlendirilir.','Anonim alan hareketi','Değişim işareti','Ölçüm düzenini karşılaştır'],
['Seçilen alandaki bekleme veya alan kullanım süresi yalnızca yeterli görüntü varsa yorumlanır.','Alan doluluğu','Süre eşiği','Yeni düzeni dene'],
['Kullanıcı tarafından onaylanan değişiklik, önceki zaman aralıklarıyla karşılaştırılır; sonuç nedensellik kanıtı değildir.','Önceki gözlemler','Karşılaştırılabilir dönem','Değişim etkisini doğrula']
];
picker.addEventListener('change',()=>{
 const row=ideas[Number(picker.value)]||ideas[0];
 for(const [id,value] of [['vo-scene-description',row[0]],['vo-scene-input',row[1]],['vo-scene-rule',row[2]],['vo-scene-action',row[3]]]){
  const node=document.getElementById(id);if(node)node.textContent=value;
 }
});
})();
