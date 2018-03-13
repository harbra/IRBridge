/*jshint esversion: 6 */
// vue-router configuration

import Vue from 'vue';
import Buefy from 'buefy';
import Router from 'vue-router';
//import Test from '../pages/Test.vue';
import SettingsNetwork from '../pages/SettingsNetwork.vue';
import SettingsBackupRestore from '../pages/SettingsBackupRestore.vue';
//import SettingsTime from '../pages/SettingsTime.vue';
import MainBookmarks from '../pages/MainBookmarks.vue';
import MainIRMonitor from '../pages/MainIRMonitor.vue';
import MainDevices from '../pages/MainDevices.vue';
import Error404 from '../pages/Error404.vue';

Vue.use(Buefy);
Vue.use(Router);

export default new Router({
  mode:'history',
  linkActiveClass:'is-active',
  routes: [
    {
      path: '/',
      redirect: '/irmonitor'
    },
    {
      path: '/bookmarks',
      component: MainBookmarks
    },
    {
      path: '/irmonitor',
      component: MainIRMonitor
    },
    {
      path: '/devices',
      component: MainDevices
    },
    {
      path: '/settings',
      redirect: '/settings/network'
    },
    {
      path: '/settings/network',
      component: SettingsNetwork
    },
    {
      path: '/settings/backup',
      component: SettingsBackupRestore
    },
    {
      path: '/404',
      component: Error404
    },
    {
      path: '*',
      redirect: '/404'
    }
  ]
});
