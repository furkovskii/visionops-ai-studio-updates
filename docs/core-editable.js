'use strict';
(()=>{
const BASE='https://uozderqllvhuiaquulhg.supabase.co';
const PUB='sb_publishable_UynYBjWSFYyR4Samj6PeXA_gBD8OmCu';
const selectors=['.brand','.navright a','.eyebrow','.hero h1 span','.hero h1 em','.hero .lead','.hero .footnote','.actions a','.section h2','.intro','.card','.card h3','.card p','.card .state','.strip','.strip h2','.strip p','.strip .btn','footer span'];
const map=new Map();
selectors.forEach((selector,i)=>document.querySelectorAll(selector).forEach((element,j)=>{
 if(j>999)return;
 const key='web_'+i+'_'+j;
 element.dataset.voCore=key;map.set(key,element);
}));
window.visionopsCoreTargets=map;
window.visionopsCoreApply=(edit)=>{
 const el=map.get(edit.target);if(!el)return;
 if(typeof edit.text_value==='string'&&el.children.length===0)el.textContent=edit.text_value;
 if(/^#[0-9a-f]{6}$/i.test(edit.text_color||''))el.style.color=edit.text_color;
 if(/^#[0-9a-f]{6}$/i.test(edit.background_color||''))el.style.backgroundColor=edit.background_color;
 if(Number.isInteger(edit.font_size)&&edit.font_size>=9&&edit.font_size<=90)el.style.fontSize=edit.font_size+'px';
 if(Number.isInteger(edit.radius)&&edit.radius>=0&&edit.radius<=60)el.style.borderRadius=edit.radius+'px';
};
(async()=>{try{
 const r=await fetch(BASE+'/rest/v1/visionops_page_edits?select=target,text_value,text_color,background_color,font_size,radius',{headers:{apikey:PUB}});
 if(!r.ok)return;
 for(const item of await r.json())window.visionopsCoreApply(item);
}catch(e){}})();
})();