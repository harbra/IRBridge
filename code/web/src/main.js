/*jshint esversion: 6 */
import Vue from 'vue';
import App from './App.vue';
import router from './router';
import TopNav from './components/TopNav.vue';
import BottomFooter from './components/BottomFooter.vue';
import {HTTP} from './plugins/axios';
import WebEvents from './plugins/webevents';

// central configuration of server-sent events
Vue.use(WebEvents, {eventsurl: (HTTP.defaults.baseURL!=undefined ? HTTP.defaults.baseURL : "") +'/events'});

Vue.component('TopNav', TopNav);

Vue.component('BottomFooter', BottomFooter);

new Vue({
  el: '#app',
  router,
  template: '<App/>',
  components: { App }
});
