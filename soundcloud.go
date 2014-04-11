package main

import (
	"encoding/json"
	"net/http"
	"net/url"
	"log"

	"launchpad.net/go-unityscopes/v1"
)

const searchCategoryTemplate = `{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "username"
  }
}`

type SoundCloudScope struct {
	BaseURI string
	ClientId string
}

type trackInfo struct {
	Title  string `json:"title"`
	Length int    `json:"length"`
	Source string `json:"source"`
}

type actionInfo struct {
	Id    string `json:"id"`
	Label string `json:"label"`
	Icon  string `json:"icon,omitempty"`
}

type user struct {
	Id        int    `json:"id"`
	Username  string `json:"username"`
	Uri       string `json:"uri"`
	AvatarUrl string `json:"avatar_url"`
}

type track struct {
	Id           int    `json:"id"`
	CreatedAt    string `json:"created_at"`
	User         user   `json:"user"`
	Streamable   bool   `json:"streamable"`
	Downloadable bool   `json:"downloadable"`

	PermalinkUrl string `json:"permalink_url"`
	PurchaseUrl  string `json:"purchase_url"`
	ArtworkUrl   string `json:"artwork_url"`
	StreamUrl    string `json:"stream_url"`
	DownloadUrl  string `json:"download_url"`
	VideoUrl     string `json:"video_url"`

	Title        string `json:"title"`
	Description  string `json:"description"`
	LabelName    string `json:"label_name"`
	Duration     int    `json:"duration"`
	License      string `json:"license"`
}

func (sc *SoundCloudScope) buildUrl(resource string, params map[string]string) string {
	query := make(url.Values)
	query.Set("client_id", sc.ClientId)
	for key, value := range params {
		query.Set(key, value)
	}
	return sc.BaseURI + resource + ".json?" + query.Encode()
}

func (sc *SoundCloudScope) get(resource string, params map[string]string, result interface{}) error {
	resp, err := http.Get(sc.buildUrl(resource, params))
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	decoder := json.NewDecoder(resp.Body)
	return decoder.Decode(result)
}

func (sc *SoundCloudScope) Search(query string, reply *scopes.SearchReply, cancelled <-chan bool) error {
	// We currently don't have any surfacing results
	if query == "" {
		return nil
	}
	var tracks []track
	if err := sc.get("/tracks", map[string]string{"q": query, "limit": "30", "order": "hotness"}, &tracks); err != nil {
		return err
	}

	cat := reply.RegisterCategory("soundcloud", "SoundCloud", "", searchCategoryTemplate)
	for _, track := range tracks {
		result := scopes.NewCategorisedResult(cat)
		result.SetURI(track.PermalinkUrl)
		result.SetTitle(track.Title)
		if track.ArtworkUrl != "" {
			result.SetArt(track.ArtworkUrl)
		} else {
			result.SetArt(track.User.AvatarUrl)
		}
		result.Set("duration", track.Duration)
		result.Set("username", track.User.Username)
		result.Set("label", track.LabelName)
		result.Set("description", track.Description)
		result.Set("stream-url", track.StreamUrl)
		result.Set("purchase-url", track.PurchaseUrl)
		result.Set("video-url", track.VideoUrl)
		if err := reply.Push(result); err != nil {
			return err
		}
	}
	return nil
}

func (sc *SoundCloudScope) Preview(result *scopes.Result, reply *scopes.PreviewReply, cancelled <-chan bool) error {
	header := scopes.NewPreviewWidget("header", "header")
	header.AddAttributeMapping("title", "title")
	header.AddAttributeMapping("subtitle", "username")

	art := scopes.NewPreviewWidget("art", "image")
	art.AddAttributeMapping("source", "art")

	title, err := result.Get("title")
	if err != nil {
		return err
	}
	duration, err := result.Get("duration")
	if err != nil {
		return err
	}
	streamUrl, err := result.Get("stream-url")
	if err != nil {
		return err
	}
	tracks := scopes.NewPreviewWidget("tracks", "audio")
	tracks.AddAttributeValue("tracks", []trackInfo{trackInfo{
		Title: title.(string),
		Length: int(duration.(float64) / 1000),
		Source: streamUrl.(string) + "?client_id=" + sc.ClientId,
	}})

	actions := scopes.NewPreviewWidget("actions", "actions")
	actions.AddAttributeValue("actions", []actionInfo{actionInfo{
		Id: "view",
		Label: "SoundCloud",
	}})

	description := scopes.NewPreviewWidget("description", "text")
	description.AddAttributeMapping("text", "description")

	return reply.PushWidgets(header, art, tracks, actions, description)
}

func main() {
	log.Println("Starting soundcloud scope")
	scope := &SoundCloudScope{
		BaseURI: "https://api.soundcloud.com",
		ClientId: "398e83f17ec3c5cf945f04772de9f400",
	}
	scopes.Run("soundcloud", scope)
}
