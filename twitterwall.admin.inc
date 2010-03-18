<?php



/**
 * Admin settings screen for TwitterWall
 */
function twitterwall_admin_settings() {
  $twitter_string = variable_get('twitterwall_string', '#drupal');

  $form['twitterwall_string'] = array(
     '#type' => 'textfield',
     '#title' => t('Twitter String'),
     '#default_value' => $twitter_string,
     '#description' => t('Please enter the search string used to retrieve from twitter.'),
  );

  $form['twitterwall_count'] = array(
    '#type' => 'select',
    '#title' => t('Tweet Count'),
    '#default_value' => variable_get('twitterwall_count', 50),
    '#options' => array(
      '10' => '10',
      '25' => '25',
      '50' => '50',
    ),
    '#description' => t('How many tweets should be retrieved'),
  );

  $period = drupal_map_assoc(array(0, 900, 1800, 3600, 5400, 7200), 'format_interval');

  $period[0] = t('Every cron run');

  $form['twitterwall_threshold'] = array(
    '#type' => 'select',
    '#title' => t('Tweet Retrieval Threshold'),
    '#default_value' => variable_get('twitterwall_threshold', 0),
    '#options' => $period,
    '#description' => t('This determines how much time to wait before new tweets are retrieved.'),
  );

  $form['twitterwall_status'] = array(
    '#type' => 'select',
    '#title' => t('Default Tweet Status'),
    '#default_value' => variable_get('twitterwall_status', 0),
    '#options' => array(
      '0' => t('Unpublished'),
      '1' => t('Published'),
    ),
    '#description' => t('Should tweets be published automatically'),
  );

  $form['twitterwall_delete'] = array(
    '#type' => 'select',
    '#title' => t('Delete the Last Tweet ID'),
    '#default_value' => variable_get('twitterwall_delete', 'no'),
    '#options' => array(
      'no' => t('No'),
      'yes' => t('Yes')
    ),
    '#description' => t('The last tweet that is retrieved is remembered to try and avoid retrieving duplicate tweets, if you change the search string you may need to set this to yes.'),
  );

  return system_settings_form($form);
}

function twitterwall_adminpage($form_state) {
  if ( isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete' ) {
    return twitterwall_multiple_delete_confirm($form_state, array_filter($form_state['values']['tweets']));
  }

  $form['admin']  = twitterwall_admin_tweets();

  return $form;
}

function twitterwall_multiple_delete_confirm(&$form_state, $tweets) {
  $form['tweets'] = array(
    '#prefix' => '<ul>',
    '#suffix' => '</ul>',
    '#tree' => TRUE
  );

  // array_filter returns only elements with TRUE values
  foreach ($tweets as $id => $value) {
    $form['tweets'][$id] = array(
      '#type' => 'hidden',
      '#value' => $id,
      '#prefix' => '<li>',
      '#suffix' => check_plain($value) ."</li>\n",
    );
  }

  $form['operation'] = array(
    '#type' => 'hidden',
    '#value' => 'delete'
  );

  $form['#submit'][] = 'twitterwall_multiple_delete_confirm_submit';

  return confirm_form(
    $form,
    t('Are you sure you want to delete these items?'),
    'admin/settings/twitterwall/admin',
    t('This action cannot be undone.'),
    t('Delete all'),
    t('Cancel')
  );
}

function twitterwall_multiple_delete_confirm_submit($form, &$form_state) {
  if ( $form_state['values']['confirm'] ) {
    foreach ( $form_state['values']['tweets'] as $id => $value ) {
      $sql = "DELETE FROM {twitterwall} WHERE id = %d";

      db_query($sql, $id);
    }

    drupal_set_message(t('The items have been deleted.'));
  }

  $form_state['redirect'] = 'admin/settings/twitterwall/admin';

  return;
}

function twitterwall_admin_tweets() {
  $path = drupal_get_path('module', 'twitterwall');

  drupal_add_js($path . '/js/twadmin.js');

  $options = array(
    'published' => t('Publish'),
    'unpublish' => t('Unpublish'),
    'delete' => t('Delete'),
  );

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $form['options']['operation'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => 'publish',
  );

  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#submit' => array('twitterwall_admin_tweets_submit'),
  );

  $headers = array(
    array('data' => ''),
    array('data' => t('Image')),
    array('data' => t('Created At'), 'field' => 'created_at_epoch', 'sort' => 'desc'),
    array('data' => t('From User'), 'field' => 'from_user'),
    array('data' => t('Message'), 'field' => 'message'),
    array('data' => t('Status'), 'field' => 'status', 'sort' => 'desc'),
  );

  $sql = "SELECT * FROM {twitterwall}";

  $sql .= tablesort_sql($headers);

  $result = pager_query($sql, variable_get('twitterwall_count', 50));

  $rows = array();

  if ( $result ) {
    while ( $row = db_fetch_array($result) ) {
      $tweets[$row['id']] = '';
      
      $profile_url = 'http://twitter.com/'.$row['from_user'] ;
      
      $form['profile_image_url'][$row['id']] = array(
        '#value' => l(theme('image', $row['profile_image_url'], $row['from_user'], '', NULL, FALSE), $profile_url, array('html'=>TRUE)),
      );
      
      $form['created_at'][$row['id']] = array(
        '#value' => $row['created_at'],
      );
      
      $form['from_user'][$row['id']] = array(
        '#value' => l($row['from_user'], $profile_url),
      );
      
      $form['message'][$row['id']] = array(
        '#value' => $row['message'],
      );

      $form['status'][$row['id']] = array(
        '#value' => ( $row['status'] == '1' ) ? t('Published') : t('Unpublished'),
      );
    }
  }

  $form['tweets'] = array(
    '#type' => 'checkboxes',
    '#options' => $tweets
  );

  $form['pager'] = array(
    '#value' => theme('pager', NULL, variable_get('twitterwall_count', 50), 0)
  );

  $form['selectall'] = array(
    '#type' => 'checkbox',
    '#title' => '',
  );

  $form['#theme'] = 'twitterwall_admin_tweets';

  return $form;
}

function theme_twitterwall_admin_tweets($form) {
  $headers = array(
    array('data' => drupal_render($form['selectall'])),
    array('data' => t('Image')),
    array('data' => t('Created At'), 'field' => 'created_at_epoch', 'sort' => 'desc'),
    array('data' => t('From User'), 'field' => 'from_user'),
    array('data' => t('Message'), 'field' => 'message'),
    array('data' => t('Status'), 'field' => 'status', 'sort' => 'desc'),
  );

  $output = '';

  $output .= drupal_render($form['options']);

  $rows = array();

  foreach ( element_children($form['message']) as $key ) {
    $row = array();

    $row[] = drupal_render($form['tweets'][$key]);
    $row[] = drupal_render($form['profile_image_url'][$key]);
    $row[] = drupal_render($form['created_at'][$key]);
    $row[] = drupal_render($form['from_user'][$key]);
    $row[] = drupal_render($form['message'][$key]);
    $row[] = drupal_render($form['status'][$key]);

    $rows[] = $row;
  }

  $output .= theme('table', $headers, $rows);

  if ( $form['pager']['#value'] ) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}

function twitterwall_admin_tweets_submit($form, &$form_state) {
  $values = $form_state['values'];

  $tweets = array_filter($values['tweets']);

  if ( is_array($tweets) && count($tweets) ) {
    $status = NULL;

    switch ( $values['operation'] ) {
      case 'unpublished' :
        $status = 0;
        $message = t('The tweets have been unpublished.');

        break;
      case 'published' :
        $status = 1;
        $message = t('The tweets have been published.');

        break;
      case 'delete' :
        $status = 2;
        $message = t('The tweets have been deleted.');

        break;
    }

    if ( $status == 0 || $status == 1 ) {
      $sql = "UPDATE {twitterwall} SET status = %d WHERE id IN (%s)";

      db_query($sql, $status, implode($tweets, ','));
    }
    else if ( $status == 2 ) {
      $sql = "DELETE FROM {twitterwall} WHERE id IN (%s)";

      db_query($sql, implode($tweets, ','));
    }
  }
  else {
    $message = t('You must select at least one tweet.');
  }

  drupal_set_message($message);
}

/**
 * We might as well follow the standard menu structure though (esp as
 * we *love* admin_menu). Can we do add this menu entry into the
 * Content => Create => menu without using the node/add path? That'd
 * be nice.
 *
 * We're also going to leave a few items blank - Twitter ID etc - as
 * these don't apply outside of Twitter. This may be an issue given
 * that the module uses that as a cache key.
 */
function twitterwall_add_tweet_form() {
  $form['#attributes']['enctype'] = 'multipart/form-data';
    
  $form['message'] = array(
    '#title' => t('Message'),
    '#type' => 'textarea',
  ) ;
    
  $form['from_user'] = array(
    '#title' => t('Your name'),
    '#type' => 'textfield',
    '#default_value' => '',
  ) ;

  global $user ;
  if ( $user->uid != 0 ) {
    $form['from_user']['#default_value'] = $user->name ;
    if ( !user_access('enter custom twitter handle') ) {
      $form['from_user']['#type'] = 'value' ;
    }
  }

  $form['profile_image'] = array(
    '#type' => 'file',
    '#title' => t('Profile Image'),
  ) ;
    
  $form['op'] = array( 
    '#type' => 'submit',
    '#value' => 'Submit',
  ) ;


  $form['#validate'][] = 'twitterwall_validate_picture';

  return $form ;

}

function twitterwall_add_tweet_form_validate($form_id, &$form_state) {
    if ( empty($form_state['values']['from_user']) ) {
        form_set_error('form_user', t('You must add a username to send this message from.'));
    }
    if ( empty($form_state['values']['message']) ) {
        form_set_error('message', t('You must add a message to post.'));
    }
    if ( empty($form_state['values']['picture']) ) {
        // we really should see if this username has associated an
        // image before here
        form_set_error('profile_image', t('You must upload an image with each tweet.'));
    }
}

/**
 * 
 */
function twitterwall_add_tweet_form_submit($form_id, &$form_state) {
  $tweet = array(
    'id' => '',
    'created_at' => date('r'),
    'profile_image_url' => $form_state['values']['picture'],
    'from_user' => $form_state['values']['from_user'],
    'from_user_id' => '',
    'to_user_id' => '',
    'text' => $form_state['values']['message'],
    'status' => 0,
  ) ;
  
  twitterwall_add_tweet($tweet, 1) ;
}

/**
 * Validate the picture. Handle upload. If imagecache is available,
 * alter the URL to include the imagecache path.
 */
function twitterwall_validate_picture(&$form, &$form_state) {
  $validators = array(
    'file_validate_is_image' => array(),
    'file_validate_image_resolution' => array(
      variable_get('twitterwall_max_picture_dimensions','800x600'), 
      variable_get('twitterwall_min_picture_dimensions', '48x48')
    ),
    'file_validate_size' => array(
      variable_get('twitterwall_max_picture_file_size', 0.5 * 1024 * 1024)
    ),
  ) ;
  if ( $file = file_save_upload('profile_image', $validators, file_directory_path()) ) {
    // The image was saved using file_save_upload() and was added to the
    // files table as a temporary file. We'll make a copy and let the garbage
    // collector delete the original upload.
    $info = image_get_info($file->filepath);

    $directory = file_create_path(variable_get('twitterwall_image_path','twitterwall')) ;
    if ( file_check_directory($directory, FILE_CREATE_DIRECTORY) ) {
      $destination = $directory .'/tweet-'. md5($form['values']['from_user']) .'.'. $info['extension'];
      if (file_copy($file, $destination, FILE_EXISTS_REPLACE)) {
        $form_state['values']['picture'] = url($destination);
      }
      else {
        form_set_error('profile_image', t("Failed to upload the profile image; the %directory directory doesn't exist or is not writable.", array('%directory' => variable_get('twitterwall_image_path', 'twitterwall'))));
      }      
    }
  }
}

