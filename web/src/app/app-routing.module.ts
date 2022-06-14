import { NgModule }             from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { LogComponent }         from './log/log.component';
import { SettingsComponent }    from './settings/settings.component';

const routes: Routes = [
  { path: 'log',      component: LogComponent },
  { path: 'settings', component: SettingsComponent }
];

@NgModule({
  declarations: [],
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
